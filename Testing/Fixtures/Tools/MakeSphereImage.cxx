// MakeSphereImage — write a tiny synthetic sphere binary image to disk.
// Used by scripts/generate-cmrep-ground-truth.sh as the seed input for the
// cmrep parity fixtures.
//
// Usage: MakeSphereImage <output.nii.gz> [N]
//
//   N (default 32) is the cube side length. The sphere is centered at
//   (N/2, N/2, N/2) with radius N/4 and pixel value 1 inside / 0 outside.
//   Spacing is 1mm isotropic; origin is 0.

#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkImageRegionIteratorWithIndex.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

int main(int argc, char *argv[])
{
  if(argc < 2)
  {
    std::fprintf(stderr, "Usage: %s <output.nii.gz> [N]\n", argv[0]);
    return 1;
  }

  using ImageType = itk::Image<float, 3>;
  const int N = (argc > 2) ? std::atoi(argv[2]) : 32;

  ImageType::Pointer img = ImageType::New();
  ImageType::SizeType sz = { { static_cast<itk::SizeValueType>(N),
                               static_cast<itk::SizeValueType>(N),
                               static_cast<itk::SizeValueType>(N) } };
  ImageType::IndexType ix = { { 0, 0, 0 } };
  img->SetRegions(ImageType::RegionType(ix, sz));
  ImageType::SpacingType s; s.Fill(1.0);
  img->SetSpacing(s);
  img->Allocate();

  const double c = N / 2.0, r = N / 4.0;
  itk::ImageRegionIteratorWithIndex<ImageType> it(img, img->GetBufferedRegion());
  for(; !it.IsAtEnd(); ++it)
  {
    auto i = it.GetIndex();
    double dx = i[0] - c, dy = i[1] - c, dz = i[2] - c;
    it.Set(std::sqrt(dx * dx + dy * dy + dz * dz) < r ? 1.0f : 0.0f);
  }

  itk::ImageFileWriter<ImageType>::Pointer w = itk::ImageFileWriter<ImageType>::New();
  w->SetFileName(argv[1]);
  w->SetInput(img);
  w->SetUseCompression(true);
  w->Update();

  std::printf("Wrote %s (%dx%dx%d sphere, r=%.1f)\n", argv[1], N, N, N, r);
  return 0;
}
