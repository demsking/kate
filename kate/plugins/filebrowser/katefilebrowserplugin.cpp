/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2007 Mirko Stocker <me@misto.ch>
   Copyright (C) 2009 Dominik Haumann <dhaumann kde org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

//BEGIN Includes
#include "katefilebrowserplugin.h"
#include "katefilebrowserplugin.moc"
#include "katefilebrowser.h"

#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QGroupBox>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QRegExp>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>

#include <kate/mainwindow.h>
#include <ktexteditor/view.h>

#include <kaboutdata.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <kactionselector.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kdiroperator.h>
#include <kfileitem.h>
#include <khbox.h>
#include <khistorycombobox.h>
#include <kdeversion.h>
#include <kpluginfactory.h>
#include <ktoolbar.h>
#include <kurlcombobox.h>
#include <kurlcompletion.h>
//END Includes

K_PLUGIN_FACTORY(KateFileBrowserFactory, registerPlugin<KateFileBrowserPlugin>();)
K_EXPORT_PLUGIN(KateFileBrowserFactory(KAboutData("katefilebrowserplugin","katefilebrowserplugin",ki18n("Filesystem Browser"), "0.1", ki18n("Browse through the filesystem"), KAboutData::License_LGPL_V2)) )

//BEGIN KateFileBrowserPlugin
KateFileBrowserPlugin::KateFileBrowserPlugin(QObject* parent, const QList<QVariant>&)
  : Kate::Plugin ((Kate::Application*)parent)
{
}

Kate::PluginView *KateFileBrowserPlugin::createView (Kate::MainWindow *mainWindow)
{
  KateFileBrowserPluginView* kateFileSelectorPluginView = new KateFileBrowserPluginView (mainWindow);
  return kateFileSelectorPluginView;
}
//END KateFileBrowserPlugin




//BEGIN KateFileBrowserPluginView
KateFileBrowserPluginView::KateFileBrowserPluginView (Kate::MainWindow *mainWindow)
: Kate::PluginView (mainWindow)
{
  // init console
  QWidget *toolview = mainWindow->createToolView ("kate_private_plugin_katefileselectorplugin", Kate::MainWindow::Left, SmallIcon("document-open"), i18n("Filesystem Browser"));
  m_fileSelector = new KateFileBrowser(mainWindow, toolview);
}

KateFileBrowserPluginView::~KateFileBrowserPluginView ()
{
  // cleanup, kill toolview + console
  delete m_fileSelector->parentWidget();
}

void KateFileBrowserPluginView::readSessionConfig(KConfigBase* config, const QString& group)
{
  m_fileSelector->readSessionConfig(config, group);
}

void KateFileBrowserPluginView::writeSessionConfig(KConfigBase* config, const QString& group)
{
  m_fileSelector->writeSessionConfig(config, group);
}
//ENDKateFileBrowserPluginView


// kate: space-indent on; indent-width 2; replace-tabs on;

