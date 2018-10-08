#include "../../inc/ui.h"


#define  printDebugMsg()     qDebug() << __FILE__ << __LINE__

MyEventLoop::MyEventLoop(QObject *parent)
                    : QObject(parent)
{
    runflag = false;
}

MyEventLoop::~MyEventLoop()
{
    runflag = false;
}

int MyEventLoop::exec(QEventLoop::ProcessEventsFlags flags,bool hassignal)
{
    if (runflag)
    {
        qDebug()<<"Eventloop is already running!";
        return -1;
    }

    runflag = true;

    if(hassignal)
    {
        emit enterEventLoop();
    }
    printDebugMsg();
    while(runflag)
    {
        qApp->processEvents(flags,500);
    }
    printDebugMsg();
    if(hassignal)
    {
        printDebugMsg();
        emit exitEventloop();
        printDebugMsg();
    }
    printDebugMsg();
    return 0;
}
void MyEventLoop::exit()
{  
    printDebugMsg();
    runflag = false;
}

bool MyEventLoop::isRunning() const
{
    printDebugMsg();
    qDebug()<<"runflag:"<<runflag;
    return runflag;
}
void MyEventLoop::quit()
{
    printDebugMsg();
    runflag = false;
}
