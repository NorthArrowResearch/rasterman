# RasterManager

Raster manager is a tool created by North Arrow Research. It is designed to provide rock-solid operations for manipulating rasters.

It does not depend on any of our other repositories.

## Folder structure:

In order to build this project alongside our [Habitat Model](https://bitbucket.org/northarrowresearch/habitat-model-console) and [GCD](https://bitbucket.org/northarrowresearch/gcd-console) tools you will need the following directory structure so that everything else can find rastermanager

```
<PROJECTROOT>/
<PROJECTROOT>/HabitatModel/habitatmodel      <-- This repo's root
<PROJECTROOT>/GCD/gcd-console/               <-- The GCD console repo root
<PROJECTROOT>/RasterManager/rastermanager/   <-- The GCD console repo root
<PROJECTROOT>/Release/Debug(32/64)           <-- Debug executable for all projects
<PROJECTROOT>/Deploy/Release(32/64)          <-- Release executables for all projects
```
