/*
  This file is part of the KDE Help Center
 
  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA  02110-1301, USA
*/

#include "khc_indexbuilder.h"

#include "version.h"

#include <kaboutdata.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kuniqueapplication.h>
#include <kdebug.h>
#include <k3process.h>
#include <kconfig.h>

#include <QFile>
#include <QTextStream>
#include <QDBusMessage>
#include <QDBusConnection>

#include <unistd.h>
#include <stdlib.h>
#include <iostream>

using namespace KHC;

IndexBuilder::IndexBuilder()
{
  kDebug(1402) << "IndexBuilder()" << endl;
}

void IndexBuilder::buildIndices( const QString &cmdFile )
{
  QFile f( cmdFile );
  if ( !f.open( QIODevice::ReadOnly ) ) {
    kError() << "Unable to open file '" << cmdFile << "'" << endl;
    exit( 1 );
  }
  kDebug(1402) << "Opened file '" << cmdFile << "'" << endl;
  QTextStream ts( &f );
  QString line = ts.readLine();
  while ( !line.isNull() ) {
    kDebug(1402) << "LINE: " << line << endl;
    mCmdQueue.append( line );
    line = ts.readLine();
  }

  processCmdQueue();
}

void IndexBuilder::processCmdQueue()
{
  kDebug(1402) << "IndexBuilder::processCmdQueue()" << endl;

  QStringList::Iterator it = mCmdQueue.begin();

  if ( it == mCmdQueue.end() ) {
    quit();
    return;
  }

  QString cmd = *it;

  kDebug(1402) << "PROCESS: " << cmd << endl;

  K3Process *proc = new K3Process;
  proc->setRunPrivileged( true );

  QStringList args = cmd.split( " ");
  *proc << args;

  connect( proc, SIGNAL( processExited( K3Process * ) ),
           SLOT( slotProcessExited( K3Process * ) ) );
  connect( proc, SIGNAL( receivedStdout(K3Process *, char *, int ) ),
           SLOT( slotReceivedStdout(K3Process *, char *, int ) ) );
  connect( proc, SIGNAL( receivedStderr(K3Process *, char *, int ) ),
           SLOT( slotReceivedStderr(K3Process *, char *, int ) ) );

  mCmdQueue.erase( it );

  if ( !proc->start( K3Process::NotifyOnExit, K3Process::AllOutput ) ) {
    sendErrorSignal( i18n("Unable to start command '%1'.", cmd ) );
    processCmdQueue();
    delete proc;
  }
}

void IndexBuilder::slotProcessExited( K3Process *proc )
{
  kDebug(1402) << "IndexBuilder::slotIndexFinished()" << endl;

  if ( !proc->normalExit() ) {
    kError(1402) << "Process failed" << endl;
  } else {
    int status = proc->exitStatus();
    kDebug(1402) << "Exit status: " << status << endl;
  }

  delete proc;

  sendProgressSignal();

  processCmdQueue();
}

void IndexBuilder::slotReceivedStdout( K3Process *, char *buffer, int buflen )
{
  QString text = QString::fromLocal8Bit( buffer, buflen );
  std::cout << text.toLocal8Bit().data() << std::flush;
}

void IndexBuilder::slotReceivedStderr( K3Process *, char *buffer, int buflen )
{
  QString text = QString::fromLocal8Bit( buffer, buflen );
  std::cerr << text.toLocal8Bit().data() << std::flush;
}

void IndexBuilder::sendErrorSignal( const QString &error )
{
  kDebug(1402) << "IndexBuilder::sendErrorSignal()" << endl;
  QDBusMessage message =
     QDBusMessage::createSignal("/kcmhelpcenter", "org.kde.kcmhelpcenter", "buildIndexError");
  message <<error;
  QDBusConnection::sessionBus().send(message);

}

void IndexBuilder::sendProgressSignal()
{
  kDebug(1402) << "IndexBuilder::sendProgressSignal()" << endl;
  QDBusMessage message =
        QDBusMessage::createSignal("/kcmhelpcenter", "org.kde.kcmhelpcenter", "buildIndexProgress");
  QDBusConnection::sessionBus().send(message);
}

void IndexBuilder::quit()
{
  kDebug(1402) << "IndexBuilder::quit()" << endl;

  qApp->quit();
}


static KCmdLineOptions options[] =
{
  { "+cmdfile", I18N_NOOP("Document to be indexed"), 0 },
  { "+indexdir", I18N_NOOP("Index directory"), 0 },
  KCmdLineLastOption
};

int main( int argc, char **argv )
{
  KAboutData aboutData( "khc_indexbuilder",
                        I18N_NOOP("KHelpCenter Index Builder"),
                        HELPCENTER_VERSION,
                        I18N_NOOP("The KDE Help Center"),
                        KAboutData::License_GPL,
                        I18N_NOOP("(c) 2003, The KHelpCenter developers") );

  aboutData.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );

  // Note: no KComponentData seems necessary
  QCoreApplication app( *KCmdLineArgs::qt_argc(), *KCmdLineArgs::qt_argv() );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  if ( args->count() != 2 ) {
    kDebug(1402) << "Wrong number of arguments." << endl;
    return 1;
  }

  QString cmdFile = args->arg( 0 );
  QString indexDir = args->arg( 1 );

  kDebug(1402) << "cmdFile: " << cmdFile << endl;
  kDebug(1402) << "indexDir: " << indexDir << endl;

  QFile file( indexDir + "/testaccess" );
  if ( !file.open( QIODevice::WriteOnly ) || !file.putChar( ' ' ) ) {
    kDebug(1402) << "access denied" << endl;
    return 2;
  } else {
    kDebug(1402) << "can access" << endl;
    file.remove();
  }

  IndexBuilder builder;

  builder.buildIndices( cmdFile );

  return app.exec();
}

#include "khc_indexbuilder.moc"

// vim:ts=2:sw=2:et
