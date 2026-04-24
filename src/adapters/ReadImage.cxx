#include "adapters/ReadImage.h"

#include "io/ImageIO.h"

template <class TPixel, unsigned int VDim>
void ReadImage<TPixel, VDim>::operator()(const std::string &filename)
{
  ImagePointer img = ImageIO<TPixel, VDim>::Read(filename);
  auto size = img->GetLargestPossibleRegion().GetSize();
  c->Verbose() << "Reading image #" << (c->m_Stack.size() + 1) << " from "
               << filename << " (size " << size << ")" << std::endl;
  c->m_Stack.PushImage(img);
}

template class ReadImage<float, 3>;
