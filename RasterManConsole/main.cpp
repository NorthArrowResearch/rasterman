#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include "rastermanager_exception.h"
#include "rastermanager_interface.h"
#include "rastermanengine.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    try
    {
        RasterManager::RasterManEngine * rm = new RasterManager::RasterManEngine();
        int eResult = rm->Run(argc, argv);
        if (eResult <= 0) {
            std::cout << std::endl <<  RasterManager::RasterManagerException::GetReturnCodeOnlyAsString(eResult)<< std::endl;
        }
        else {
            std::cerr << std::endl << "Error: " <<  RasterManager::RasterManagerException::GetReturnCodeOnlyAsString(eResult)<< std::endl;
            exit (EXIT_FAILURE);
        }
        exit (EXIT_SUCCESS);
    }
    catch (RasterManager::RasterManagerException & e)
    {
        std::cerr << "Error: " << e.GetReturnMsgAsString().toStdString() << std::endl;
        exit (EXIT_FAILURE);
    }
    catch (std::exception & e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        exit (EXIT_FAILURE);
    }
    //return a.exec();
    qApp->quit();
}
