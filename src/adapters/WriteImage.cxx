#include "adapters/WriteImage.h"

#include "io/ImageIO.h"

template <class TPixel, unsigned int VDim>
void WriteImage<TPixel, VDim>::operator()(const std::string &filename)
{
  if(c->m_Stack.empty()) throw StackAccessException();
  const Item &top = c->m_Stack.back();
  if(!top.IsImage())
    throw TypeMismatchException("image", top.KindName());

  c->Verbose() << "Writing image #" << c->m_Stack.size() << " to "
               << filename << std::endl;
  ImageIO<TPixel, VDim>::Write(top.image, filename);
}

template class WriteImage<float, 3>;
