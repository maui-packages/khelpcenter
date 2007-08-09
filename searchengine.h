#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <QObject>


#include <kio/job.h>

#include "docentrytraverser.h"

class K3Process;

namespace KHC {

class Formatter;
class SearchEngine;
class View;
class SearchHandler;

class SearchTraverser : public QObject, public DocEntryTraverser
{
    Q_OBJECT
  public:
    SearchTraverser( SearchEngine *engine, int level );
    ~SearchTraverser();

    void process( DocEntry * );
    
    void startProcess( DocEntry * );

    DocEntryTraverser *createChild( DocEntry * );

    DocEntryTraverser *parentTraverser();

    void deleteTraverser();

    void finishTraversal();

  protected:
    void connectHandler( SearchHandler *handler );
    void disconnectHandler( SearchHandler *handler );

  protected Q_SLOTS:
    void showSearchResult( SearchHandler *, DocEntry *, const QString &result );
    void showSearchError( SearchHandler *, DocEntry *, const QString &error );

  private:
    const int mMaxLevel;
  
    SearchEngine *mEngine;
    int mLevel;

    DocEntry *mEntry;
    QString mJobData;
    
    QString mResult;
    
    QMap<SearchHandler *, int> mConnectCount;
};


class SearchEngine : public QObject
{
    Q_OBJECT
  public:
    enum Operation { And, Or };

    SearchEngine( View * );
    ~SearchEngine();

    bool initSearchHandlers();

    bool search( const QString & words, const QString & method = "and", int matches = 5,
                 const QString & scope = "" );

    Formatter *formatter() const; 
    View *view() const;

    QString substituteSearchQuery( const QString &query );

    static QString substituteSearchQuery( const QString &query,
      const QString &identifier, const QStringList &words, int maxResults,
      Operation operation, const QString &lang );

    void finishSearch();

    /**
      Append error message to error log.
    */
    void logError( DocEntry *entry, const QString &msg );

    /**
      Return error log.
    */
    QString errorLog() const;

    bool isRunning() const;

    SearchHandler *handler( const QString &documentType ) const;

    QStringList words() const;
    int maxResults() const;
    Operation operation() const;

    bool canSearch( DocEntry * );
    bool needsIndex( DocEntry * );

  Q_SIGNALS:
    void searchFinished();

  protected Q_SLOTS:
    void searchStdout(K3Process *proc, char *buffer, int buflen);
    void searchStderr(K3Process *proc, char *buffer, int buflen);
    void searchExited(K3Process *proc);

  protected:
    void processSearchQueue();
    
  private:
    K3Process *mProc;
    bool mSearchRunning;
    QString mSearchResult;

    QString mStderr;

    View *mView;
    
    QString mWords;
    int mMatches;
    QString mMethod;
    QString mLang;
    QString mScope;

    QStringList mWordList;
    int mMaxResults;
    Operation mOperation;
    
    DocEntryTraverser *mRootTraverser;

    QMap<QString, SearchHandler *> mHandlers;
};

}

#endif
// vim:ts=2:sw=2:et
