/* This file is part of the KDE libraries
   Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
   Copyright (C) 2002-2004 Christoph Cullmann <cullmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __KATE_BUFFER_H__
#define __KATE_BUFFER_H__

#include "katetextline.h"
#include "katecodefoldinghelpers.h"

#include <kvmallocator.h>

#include <qptrlist.h>
#include <qobject.h>
#include <qtimer.h>

class KateLineInfo;
class KateDocument;
class KateHighlighting;
class KateBufBlockList;
class KateBuffer;

class QTextCodec;

/**
 * The KateBufBlock class contains an amount of data representing
 * a certain number of lines.
 */
class KateBufBlock
{
  friend class KateBufBlockList;

  public:
    /**
     * Create an empty block. (empty == ONE line)
     */
    KateBufBlock ( KateBuffer *parent, KateBufBlock *prev = 0, KateBufBlock *next = 0,
                   QTextStream *stream = 0, bool lastCharEOL = false, bool *eof = 0 );
    
    /**
     * destroy this block and take care of freeing all mem
     */
    ~KateBufBlock ();
    
  private:
    /**
     * fill the block with the lines from the given stream
     * lastCharEOL indicates if a trailing newline should be
     * appended at the end
     * internal there will be a limit for the taken lines per block
     * returns EOL as bool
     */
    bool fillBlock (QTextStream *stream, bool lastCharEOL);
    
  public:
    /**
     * state flags
     */
    enum State
    {
      stateSwapped = 0,
      stateClean = 1,
      stateDirty = 2
    };
    
    /**
     * returns the current state of this block
     */
    State state () const { return m_state; }
    
  public:
    /**
     * return line @p i
     * The first line of this block is line 0.
     * if you modifiy this line, please mark the block as dirty
     */
    KateTextLine::Ptr line(uint i);
    
    /**
     * insert @p line in front of line @p i
     * marks the block dirty
     */
    void insertLine(uint i, KateTextLine::Ptr line);
    
    /**
     * remove line @p i
     * marks the block dirty
     */
    void removeLine(uint i);
    
    /**
     * mark this block as dirty, will invalidate the swap data
     * insert/removeLine will mark the block dirty itself
     */
    void markDirty ();
  
  public:
    /**
     * first line in block
     */
    inline uint startLine () const { return m_startLine; };
    
    /**
     * update the first line, needed to keep it up to date
     */
    inline void setStartLine (uint line) { m_startLine = line; }
    
    /**
     * first line behind this block
     */
    inline uint endLine () const { return m_startLine + m_lines; }
    
    /**
     * lines in this block
     */
    inline uint lines () const { return m_lines; }

    /**
     * prev/next block
     */
    inline KateBufBlock *prev () { return m_prev; }
    inline KateBufBlock *next () { return m_next; }
  
  /**
   * methodes to swap in/out
   */
  private:
    /**
     * swap in the kvmallocater data, create string list
     */
    void swapIn ();

    /**
     * swap our string list out, delete it !
     */
    void swapOut ();
            
  private:
    /**
     * VERY IMPORTANT, state of this block
     * this uchar indicates if the block is swapped, loaded, clean or dirty
     */
    KateBufBlock::State m_state;
    
    /**
     * IMPORTANT, start line + lines in block
     */
    uint m_startLine;
    uint m_lines;

    /**
     * here we swap our stuff
     */
    KVMAllocator::Block *m_vmblock;
    uint m_vmblockSize;

    /**
     * list of textlines
     */
    KateTextLine::List m_stringList;

    /**
     * parent buffer.
     */
    KateBuffer* m_parent;

    /**
     * prev/next block
     */
    KateBufBlock *m_prev;
    KateBufBlock *m_next;
    
  private:
    /**
     * list pointer, to which list I belong
     * list element pointers for the KateBufBlockList ONLY !!!
     */
    KateBufBlockList *list;
    KateBufBlock *listPrev;
    KateBufBlock *listNext;
};

/**
 * list which allows O(1) inserts/removes
 * will not delete the elements on remove
 * will use the next/prevNode pointers in the KateBufBlocks !
 * internal use: loaded/clean/dirty block lists
 */
class KateBufBlockList
{
  public:
    KateBufBlockList ();
    ~KateBufBlockList ();
    
  public:
    /**
     * count of blocks in this list
     */
    inline uint count() const { return m_count; }
  
    /**
     * first block in this list or 0
     */
    inline KateBufBlock *first () { return m_first; };
    
    /**
     * last block in this list or 0
     */
    inline KateBufBlock *last () { return m_last; };

    /**
     * is buf the last block ?
     */
    inline bool isFirst (KateBufBlock *buf) { return m_first == buf; };
    
    /**
     * is buf the last block ?
     */
    inline bool isLast (KateBufBlock *buf) { return m_last == buf; };
        
    /**
     * append a block to this list !
     * will remove it from the list it belonged before !
     */
    void append (KateBufBlock *buf);
    
    /**
     * remove the block from the list it belongs to !
     */
    inline static void remove (KateBufBlock *buf)
    {
      if (buf->list)
        buf->list->removeInternal (buf);
    }
    
  private:
    void removeInternal (KateBufBlock *buf);
    
  private:
    uint m_count;
    KateBufBlock *m_first;
    KateBufBlock *m_last;
};

/**
 * The KateBuffer class maintains a collections of lines.
 * It allows to maintain state information in a lazy way.
 * It handles swapping out of data using secondary storage.
 *
 * It is designed to handle large amounts of text-data efficiently
 * with respect to CPU and memory usage.
 *
 * @author Waldo Bastian <bastian@kde.org>
 * @author Christoph Cullmann <cullmann@kde.org>
 */
class KateBuffer : public QObject
{
  Q_OBJECT
  
  friend class KateBufBlock;

  public:
    /**
     * Create an empty buffer.
     */
    KateBuffer (KateDocument *doc);

    /**
     * Goodbye buffer
     */
    ~KateBuffer ();
    
  public:
    void editStart ();
    void editEnd ();
    
  private:
    void editTagLine (uint line);
    void editRemoveTagLine (uint line);
    void editInsertTagLine (uint line);
  
  private:
    uint editSessionNumber;
    bool editIsRunning;
    uint editTagLineStart;
    uint editTagLineEnd;
    
  public:
    /**
     * Clear the buffer.
     */
    void clear();
  
    /**
     * Open a file, use the given filename + codec (internal use of qtextstream)
     */
    bool openFile (const QString &m_file);
    
    /**
     * was the last loading broken because of not enough tmp disk space ?
     * (will be reseted on successful save of the file, user gets warning if he really wants to do it)
     */
    bool loadingBorked () const { return m_loadingBorked; }

    /**
     * Can the current codec handle all chars
     */
    bool canEncode ();

    /**
     * Save the buffer to a file, use the given filename + codec + end of line chars (internal use of qtextstream)
     */
    bool saveFile (const QString &m_file);
    
  public:
    /**
     * Return line @p i
     */
    KateTextLine::Ptr line(uint i);

    /**
     * Return line @p i without triggering highlighting
     */
    KateTextLine::Ptr plainLine(uint i);
    
    /**
     * Return the total number of lines in the buffer.
     */
    inline uint count() const { return m_lines; }
    
  private:
    /**
     * Find the block containing line @p i
     * index pointer gets filled with index of block in m_blocks
     * index only valid if returned block != 0 !
     */
    KateBufBlock *findBlock (uint i, uint *index = 0);
    
  public:
    /**
     * Mark line @p i as changed !
     */
    void changeLine(uint i);
  
    /**
     * Insert @p line in front of line @p i
     */
    void insertLine(uint i, KateTextLine::Ptr line);

    /**
     * Remove line @p i
     */
    void removeLine(uint i);
    
  public:
    inline uint countVisible () { return m_lines - m_regionTree.getHiddenLinesCount(m_lines); }
    
    inline uint lineNumber (uint visibleLine) { return m_regionTree.getRealLine (visibleLine); }
    
    inline uint lineVisibleNumber (uint line) { return m_regionTree.getVirtualLine (line); }
    
    inline void lineInfo (KateLineInfo *info, unsigned int line) { m_regionTree.getLineInfo(info,line); }

    inline uint tabWidth () const { return m_tabWidth; }
    
    inline KVMAllocator *vm () { return &m_vm; }
    
  public:
    void setTabWidth (uint w);  

    /**
     * Use @p highlight for highlighting
     *
     * @p highlight may be 0 in which case highlighting
     * will be disabled.
     */
    void setHighlight (KateHighlighting *highlight);

    KateHighlighting *highlight () { return m_highlight; };
    
    /**
     * Invalidate highlighting of whole buffer.
     */
    void invalidateHighlighting();
    
    KateCodeFoldingTree *foldingTree () { return &m_regionTree; };
    
  public slots:
    /**
     * change the visibility of a given line
     */
    void setLineVisible (unsigned int lineNr, bool visible);
  
  private:
    /**
     * Highlight information needs to be updated.
     *
     * @param buf The buffer being processed.
     * @param startState highlighting state of last line before range
     * @param from first line in range
     * @param to last line in range
     * @param invalidat should the rehighlighted lines be tagged ?
     *
     * @returns true when the highlighting in the next block needs to be updated,
     * false otherwise.
     */
    bool doHighlight (KateBufBlock *buf, uint from, uint to, bool invalidate);
    
  signals:
    /**
     * Emittend if codefolding returned with a changed list
     */
    void codeFoldingUpdated();

    /**
     * Emitted when the highlighting of a certain range has
     * changed.
     */
    void tagLines(int start, int end);

  private:
    /**
     * document we belong too
     */
    KateDocument *m_doc;
  
    /**
     * current line count
     */
    uint m_lines;
    
    /**
     * ALL blocks
     * in order of linenumbers
     */
    QValueVector<KateBufBlock*> m_blocks;
    
    /**
     * last block where the start/end line is in sync with real life
     */
    uint m_lastInSyncBlock;
    
    /**
     * last block found by findBlock, there to make searching faster
     */
    uint m_lastFoundBlock;

    /**
     * vm allocator
     */
    KVMAllocator m_vm;

    /**
     * status of the cache read/write errors
     * write errors get handled, read errors not really atm
     */
    bool m_cacheReadError;
    bool m_cacheWriteError;
    
    /**
     * had we cache error while loading ?
     */
    bool m_loadingBorked;

  /**
   * highlighting & folding relevant stuff
   */
  private:
    /**
     * current highlighting mode or 0
     */
    KateHighlighting *m_highlight;
    
    /**
     * folding tree
     */
    KateCodeFoldingTree m_regionTree;
    
    // for the scrapty indent sensitive langs
    uint m_tabWidth;
        
    uint m_lineHighlightedMax;
    uint m_lineHighlighted;
  
  /**
   * only used from the KateBufBlocks !
   */
  private:
    /**
     * all not swapped blocks !
     */
    KateBufBlockList m_loadedBlocks;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
