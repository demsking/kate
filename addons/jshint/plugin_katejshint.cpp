/***************************************************************************
                           plugin_katejshint.cpp - checks XML files using xmllint
                           -------------------
	begin                : 2002-07-06
	copyright            : (C) 2002 by Daniel Naber
	email                : daniel.naber@t-online.de
 ***************************************************************************/

/***************************************************************************
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ***************************************************************************/

/*
-fixme: show dock if "Validate XML" is selected (doesn't currently work when Kate
 was just started and the dockwidget isn't yet visible)
-fixme(?): doesn't correctly disappear when deactivated in config
*/

//TODO:
// Cleanup unneeded headers
// Find resourses and translate i18n messages
// all translations were deleted in https://websvn.kde.org/?limit_changes=0&view=revision&revision=1433517
// What to do with catalogs? What is it for?
// Implement hot key shoutcut to do xml validation
// Remove copyright above due to author orphoned this plugin?
// Possibility to check only well-formdness without validation
// Hide output in dock when switching to another tab
// Make ability to validate agains xml schema and then edit docbook
// Should del space in [km] strang in katejshint.desktop?
// Which variant should I choose? QUrl.adjusted(rm filename).path() or QUrl.toString(rm filename|rm schema)
// What about replace xmllint xmlstarlet or something?
// Maybe use QXmlReader to take dtds and xsds?

#include "plugin_katejshint.h"
#include <QHBoxLayout>
//#include "plugin_katejshint.moc" this goes to end

#include <qfile.h>
#include <qinputdialog.h>
#include <qregexp.h>
#include <qstring.h>
#include <qtextstream.h>
#include <kactioncollection.h>
#include <QApplication>
#include <QTreeWidget>
#include <QHeaderView>

#include <QAction>
#include <kcursor.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <QTemporaryFile>
#include <kpluginfactory.h>
#include <QProcess>

#include <QAction>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExp>
#include <QStandardPaths>
#include <QString>
#include <QUrl>
#include <QVBoxLayout>

#include <ktexteditor/editor.h>
#include <ktexteditor/mainwindow.h>

#include <kxmlguifactory.h>

K_PLUGIN_FACTORY_WITH_JSON(PluginKateJsHintFactory,
                           "katejshint.json",
                           registerPlugin<PluginKateJsHint>();)


PluginKateJsHint::PluginKateJsHint( QObject * const parent, const QVariantList& )
	: KTextEditor::Plugin(parent)
{
    qDebug() << "PluginXmlCheck()";
}


PluginKateJsHint::~PluginKateJsHint()
{
}


QObject *PluginKateJsHint::createView(KTextEditor::MainWindow *mainWindow)
{
    return new PluginKateJsHintView(this, mainWindow);
}


//---------------------------------
PluginKateJsHintView::PluginKateJsHintView( KTextEditor::Plugin *plugin,
                                                KTextEditor::MainWindow *mainwin)
    : QObject(mainwin)
    , KXMLGUIClient()
    , m_mainWindow(mainwin)
	, m_rx("(.+): line (\\d+), col (\\d+), (.+)", Qt::CaseInsensitive)
{
	KXMLGUIClient::setComponentName(QLatin1String("katejshint"), i18n ("Kate JSHint")); // where i18n resources?
    setXMLFile(QLatin1String("ui.rc"));

    dock = m_mainWindow->createToolView(plugin, "kate_plugin_jshint_ouputview", KTextEditor::MainWindow::Bottom, QIcon::fromTheme("misc"), i18n("JSHint Checker"));
    listview = new QTreeWidget( dock );
    m_tmp_file=0;
    QAction *a = actionCollection()->addAction("jshint");
    a->setText(i18n("Validate JavaScript"));
    connect(a, SIGNAL(triggered()), this, SLOT(slotValidate()));
    // TODO?:
    //(void)  new KAction ( i18n("Indent XML"), KShortcut(), this,
    //	SLOT(slotIndent()), actionCollection(), "xml_indent" );

    listview->setFocusPolicy(Qt::NoFocus);
    QStringList headers;
    headers << i18n("#");
    headers << i18n("Line");
    headers << i18n("Column");
    headers << i18n("Message");
    headers << i18n("File");
    listview->setHeaderLabels(headers);
    listview->setRootIsDecorated(false);
    connect(listview, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(slotClicked(QTreeWidgetItem*,int)));

    QHeaderView *header = listview->header();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);

/* TODO?: invalidate the listview when document has changed
   Kate::View *kv = application()->activeMainWindow()->activeView();
   if( ! kv ) {
   qDebug() << "Warning: no Kate::View";
   return;
   }
   connect(kv, SIGNAL(modifiedChanged()), this, SLOT(slotUpdate()));
*/

    connect(&m_proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotProcExited(int,QProcess::ExitStatus)));
    // we currently only want errors:
    m_proc.setProcessChannelMode(QProcess::SeparateChannels);
    // m_proc.setProcessChannelMode(QProcess::ForwardedChannels); // For Debugging. Do not use this.
    mainwin->guiFactory()->addClient(this);
}

PluginKateJsHintView::~PluginKateJsHintView()
{
    m_mainWindow->guiFactory()->removeClient( this );
    delete m_tmp_file;
    delete dock;
}

void PluginKateJsHintView::slotProcExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);

    // FIXME: doesn't work correct the first time:
    //if( m_dockwidget->isDockBackPossible() ) {
    //	m_dockwidget->dockBack();
//	}

    if (exitStatus != QProcess::NormalExit) {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, QString("1").rightJustified(4,' '));
        item->setText(3, "Validate process crashed.");
        listview->addTopLevelItem(item);
        return;
    }

    qDebug() << "slotProcExited()";

    QApplication::restoreOverrideCursor();
    delete m_tmp_file;
    QString stdout = QString::fromLocal8Bit(m_proc.readAllStandardOutput());
    m_tmp_file=0;
    listview->clear();
    
	if( ! stdout.isEmpty() ) {
        QStringList lines = stdout.split("\n", QString::SkipEmptyParts);
        int line_count = 0;

		for(QStringList::Iterator it = lines.begin(); it != lines.end(); ++it) {
			line_count++;

            QString line = *it;

			if (m_rx.indexIn(line, 0) != -1) {
				QString filename = m_rx.cap(1);
				QString linenumber = m_rx.cap(2);
				QString colnumber = m_rx.cap(3);
				QString message = m_rx.cap(4);

				QTreeWidgetItem *item = new QTreeWidgetItem();
				
				item->setText(0, QString::number(line_count).rightJustified(4,' '));
				item->setText(1, linenumber);
				item->setTextAlignment(1, (item->textAlignment(1) & ~Qt::AlignHorizontal_Mask) | Qt::AlignRight);
				item->setText(2, colnumber);
				item->setTextAlignment(2, (item->textAlignment(2) & ~Qt::AlignHorizontal_Mask) | Qt::AlignRight);
				item->setText(3, message);
				item->setText(4, filename);
				
				listview->addTopLevelItem(item);
			}
        }
    }
}


void PluginKateJsHintView::slotClicked(QTreeWidgetItem *item, int column)
{
	Q_UNUSED(column);
	qDebug() << "slotClicked";
	if( item ) {
		bool ok = true;
		uint line = item->text(1).toUInt(&ok);
		bool ok2 = true;
		uint column = item->text(2).toUInt(&ok);
		if( ok && ok2 ) {
			KTextEditor::View *kv = m_mainWindow->activeView();
			if( ! kv )
				return;

			kv->setCursorPosition(KTextEditor::Cursor (line-1, column));
		}
	}
}


void PluginKateJsHintView::slotUpdate()
{
	qDebug() << "slotUpdate() (not implemented yet)";
}


bool PluginKateJsHintView::slotValidate()
{
	qDebug() << "slotValidate()";

	m_mainWindow->showToolView (dock);

	KTextEditor::View *kv = m_mainWindow->activeView();
	
	if( ! kv ) {
	  return false;
	}

	delete m_tmp_file;
	
	m_tmp_file = new QTemporaryFile();
	
	if( !m_tmp_file->open() ) {
		qDebug() << "Error (slotValidate()): could not create '" << m_tmp_file->fileName() << "': " << m_tmp_file->errorString();
		KMessageBox::error(0, i18n("<b>Error:</b> Could not create "
			"temporary file '%1'.", m_tmp_file->fileName()));
		delete m_tmp_file;
		m_tmp_file=0L;

		return false;
	}

	QTextStream s ( m_tmp_file );
	s << kv->document()->text();
	s.flush();

    QString exe = QStandardPaths::findExecutable("jshint");
	if( exe.isEmpty() ) {
		exe = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, "jshint");
	}

    qDebug() << "exe=" <<exe;

	QStringList args;
	QUrl url = kv->document()->url();

	if ( ! url.isLocalFile() ) {
		qDebug() << "Error (slotValidate()): could not validate non local file '" << m_tmp_file->fileName() << "': " << m_tmp_file->errorString();
		KMessageBox::error(0, i18n("<b>Error:</b> Could not validate non local file"));
		delete m_tmp_file;
		m_tmp_file=0L;

		return false;
	}

	QString path = kv->document()->url().toString().replace(0, 7, "");

	args << path;

	// heuristic: assume that the doctype is in the first 10,000 bytes:
	// QString text_start = kv->document()->text().left(10000);
	// remove comments before looking for doctype (as a doctype might be commented out
	// and needs to be ignored then):
	// QRegExp re("<!--.*-->");
	// re.setMinimal(true);
	// text_start.remove(re);
	// QRegExp re_doctype("<!DOCTYPE\\s+(.*)\\s+(?:PUBLIC\\s+[\"'].*[\"']\\s+[\"'](.*)[\"']|SYSTEM\\s+[\"'](.*)[\"'])", Qt::CaseInsensitive);
	// re_doctype.setMinimal(true);

	// if( re_doctype.indexIn(text_start) != -1 ) {
	// 	QString dtdname;
	// 	if( ! re_doctype.cap(2).isEmpty() ) {
	// 		dtdname = re_doctype.cap(2);
	// 	} else {
	// 		dtdname = re_doctype.cap(3);
	// 	}
	// 	if( !dtdname.startsWith("http:") ) {		// todo: u_dtd.isLocalFile() doesn't work :-(
	// 		// a local DTD is used
	// 		m_validating = true;
    //                     args << "--valid";
	// 	} else {
	// 		m_validating = true;
    //                     args << "--valid";
	// 	}
	// } else if( text_start.indexOf("<!DOCTYPE") != -1 ) {
	// 	// DTD is inside the XML file
	// 	m_validating = true;
    //             args << "--valid";
	// }

	// args << m_tmp_file->fileName();
	// qDebug() << "m_tmp_file->fileName()=" << m_tmp_file->fileName();

	m_proc.start(exe, args);
	qDebug() << "m_proc.program():" << m_proc.program(); // I want to see parmeters
	qDebug() << "args=" << args;
	qDebug() << "exit code:" << m_proc.exitCode();

	if( ! m_proc.waitForStarted(-1) ) {
		KMessageBox::error(0, i18n("<b>Error:</b> Failed to execute jshint. Please make "
			"sure that jshint is installed. npm install -g jshint."));
		return false;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	return true;
}

#include "plugin_katejshint.moc"
