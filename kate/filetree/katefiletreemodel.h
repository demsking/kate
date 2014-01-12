/* This file is part of the KDE project
   Copyright (C) 2010 Thomas Fjellstrom <thomas@fjellstrom.ca>

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

#ifndef KATEFILETREEMODEL_H
#define KATEFILETREEMODEL_H

#include <QAbstractItemModel>
#include <QColor>

#include <ktexteditor/modificationinterface.h>
namespace KTextEditor {
  class Document;
}

class ProxyItem;
class ProxyItemDir;

QDebug operator<<(QDebug dbg, ProxyItem *item);
QDebug operator<<(QDebug dbg, ProxyItemDir *item);

class KateFileTreeModel : public QAbstractItemModel
{
  Q_OBJECT

  public:
    enum { DocumentRole = Qt::UserRole + 1, PathRole, OpeningOrderRole, DocumentTreeRole };

    KateFileTreeModel(QObject *p);
    virtual ~KateFileTreeModel();

    virtual Qt::ItemFlags flags( const QModelIndex & index ) const;
    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const;

    virtual QModelIndex parent( const QModelIndex & index ) const;
    virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual bool hasChildren( const QModelIndex & parent = QModelIndex() ) const;

    QModelIndex docIndex(const KTextEditor::Document *) const;

    bool isDir(const QModelIndex &index) const;

    bool listMode() const;
    void setListMode(bool);

    bool shadingEnabled() const;
    void setShadingEnabled(bool);

    const QColor &editShade() const;
    void setEditShade(const QColor &);

    const QColor &viewShade() const;
    void setViewShade(const QColor &);

    bool showFullPathOnRoots(void) const;
    void setShowFullPathOnRoots(bool);

    void documentsOpened(const QList<KTextEditor::Document *> &);
    /* used strictly for the item coloring */
    void documentActivated(const KTextEditor::Document *);
    void documentEdited(const KTextEditor::Document *);

  public Q_SLOTS:
    void documentOpened(KTextEditor::Document *);
    void documentClosed(KTextEditor::Document *);
    void documentNameChanged(KTextEditor::Document *);
    void documentModifiedChanged( KTextEditor::Document *);
    void documentModifiedOnDisc(KTextEditor::Document*, bool, KTextEditor::ModificationInterface::ModifiedOnDiskReason);
    void slotAboutToDeleteDocuments(const QList<KTextEditor::Document *> &);
    void slotDocumentsDeleted(const QList<KTextEditor::Document *> &);

  Q_SIGNALS:
    void triggerViewChangeAfterNameChange();

  private:
    ProxyItemDir *findRootNode(const QString &name, const int r = 1) const;
    ProxyItemDir *findChildNode(const ProxyItemDir *parent, const QString &name) const;
    void insertItemInto(ProxyItemDir *root, ProxyItem *item);
    void handleInsert(ProxyItem *item);
    void handleNameChange(ProxyItem *item);
    void handleEmptyParents(ProxyItemDir *item);
    void setupIcon(ProxyItem *item) const;
    void updateItemPathAndHost(ProxyItem *item) const;
    void handleDuplicitRootDisplay(ProxyItemDir *item);

    void updateBackgrounds(bool force = false);

    void initModel();
    void clearModel();
    void connectDocument(const KTextEditor::Document *);

  private:
    ProxyItemDir *m_root;
    QHash<const KTextEditor::Document *, ProxyItem *> m_docmap;

    bool m_shadingEnabled;

    QList<ProxyItem *> m_viewHistory;
    QList<ProxyItem *> m_editHistory;
    QMap<ProxyItem *, QBrush> m_brushes;

    QColor m_editShade;
    QColor m_viewShade;

    bool m_listMode;
};

#endif /* KATEFILETREEMODEL_H */

// kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;
