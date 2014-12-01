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
        if (eResult <= 0) {
            std::cout << std::endl <<  RasterManager::GetReturnCodeAsString(eResult)<< std::endl;
        }
        else {
            std::cerr << std::endl << "Error: " <<  RasterManager::GetReturnCodeAsString(eResult)<< std::endl;
            exit (EXIT_FAILURE);
        }
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
