#ifndef CONVERTMESH_IMAGE_IO_H
#define CONVERTMESH_IMAGE_IO_H

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkSmartPointer.h>

#include <string>

#include "driver/ConvertMeshException.h"

/**
 * Minimal typed image I/O wrapper. Delegates to ITK's factory-based
 * ImageFileReader / ImageFileWriter so any format registered with ITK
 * (NIfTI, NRRD, MHA, GIPL, PNG, TIFF, …) is usable.
 */
template <class TPixel, unsigned int VDim>
class ImageIO
{
public:
  typedef itk::Image<TPixel, VDim> ImageType;
  typedef itk::SmartPointer<ImageType> ImagePointer;

  static ImagePointer Read(const std::string &filename)
  {
    typedef itk::ImageFileReader<ImageType> ReaderType;
    typename ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(filename);
    try
    {
      reader->Update();
    }
    catch(itk::ExceptionObject &e)
    {
      throw ConvertMeshException("failed to read image '" + filename + "': "
                                 + e.GetDescription());
    }
    return reader->GetOutput();
  }

  static void Write(ImageType *image, const std::string &filename)
  {
    typedef itk::ImageFileWriter<ImageType> WriterType;
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(filename);
    writer->SetInput(image);
    writer->SetUseCompression(true);
    try
    {
      writer->Update();
    }
    catch(itk::ExceptionObject &e)
    {
      throw ConvertMeshException("failed to write image '" + filename + "': "
                                 + e.GetDescription());
    }
  }
};

#endif
