#include <iostream>
#include <stdio.h>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include "rastermanengine.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    try
    {
        RasterManager::RasterManEngine rm(argc, argv);
    }
    catch (std::exception& e)
    {
        std::cout << e.what();
    }

    //return a.exec();
    qApp->quit();
}
