#include "driver/ConvertMeshDriver.h"

int main(int argc, char *argv[])
{
  ConvertMeshDriver<float, 3> driver;
  return driver.ProcessCommandLine(argc, argv);
}
