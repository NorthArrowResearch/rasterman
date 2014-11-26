#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include "rastermanengine.h"
#include "rastermanager_interface.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    try
    {
        RasterManager::RasterManEngine * rm = new RasterManager::RasterManEngine();
        int eResult = rm->Run(argc, argv);
        std::cout << "\n" <<  RasterManager::GetReturnCodeAsString(eResult);
        exit (EXIT_SUCCESS);
    }
    catch (std::exception& e)
    {
        std::cerr <<"Error: " << e.what() << std::endl;
        exit (EXIT_FAILURE);
    }

    //return a.exec();
    qApp->quit();
}
