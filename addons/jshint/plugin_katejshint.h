 /***************************************************************************
                           plugin_katejshint.h
                           -------------------
	begin                : 2017-03-26
	copyright            : (C) 2017 by Sébastien Demanou
	email                : demsking@gmail.com
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

#ifndef PLUGIN_KATEJSHINT_H
#define PLUGIN_KATEJSHINT_H

#include <QProcess>
#include <QRegExp>

#include <ktexteditor/plugin.h>
#include <ktexteditor/application.h>
#include <ktexteditor/mainwindow.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

#include <QString>
#include <QVariantList>

class QTreeWidget;
class QTreeWidgetItem;
class QTemporaryFile;
class QProcess;
class QRegExp;

class PluginKateJsHintView : public QObject, public KXMLGUIClient
{
    Q_OBJECT

public:
    PluginKateJsHintView(KTextEditor::Plugin *plugin, KTextEditor::MainWindow *mainwin);
    virtual ~PluginKateJsHintView();

    KTextEditor::MainWindow *m_mainWindow;
    QWidget *dock;

public Q_SLOTS:
    bool slotValidate();
    void slotClicked(QTreeWidgetItem *item, int column);
    void slotProcExited(int exitCode, QProcess::ExitStatus exitStatus);
    void slotUpdate();

private:
    QTemporaryFile *m_tmp_file;
    KParts::ReadOnlyPart *part;
    QProcess m_proc;
    QString m_proc_stdout;
    QTreeWidget *listview;
    QRegExp m_rx;
};


class PluginKateJsHint : public KTextEditor::Plugin
{
  Q_OBJECT

public:
  explicit PluginKateJsHint( QObject* parent = 0, const QVariantList& = QVariantList() );

    virtual ~PluginKateJsHint();
     QObject *createView(KTextEditor::MainWindow *mainWindow);
};

#endif // PLUGIN_KATEJSHINT_H
