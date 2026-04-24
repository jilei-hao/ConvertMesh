#include "ConvertMeshDriver.h"

#include "adapters/ReadImage.h"
#include "adapters/ReadMesh.h"
#include "adapters/StackOps.h"
#include "adapters/WriteImage.h"
#include "adapters/WriteMesh.h"
#include "io/MeshIO.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <sstream>
#include <string>

namespace
{

std::string ToLower(std::string s)
{
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return s;
}

bool IsMeshFilename(const std::string &fn)
{
  std::string ext = ToLower(MeshIO::GetExtension(fn));
  return ext == "vtk" || ext == "vtp" || ext == "stl" || ext == "obj"
      || ext == "ply" || ext == "byu" || ext == "y";
}

bool IsImageFilename(const std::string &fn)
{
  std::string ext = ToLower(MeshIO::GetExtension(fn));
  // Common medical image extensions; delegated to ITK's factories anyway.
  return ext == "nii" || ext == "gz" || ext == "mha" || ext == "mhd"
      || ext == "nrrd" || ext == "nhdr" || ext == "gipl" || ext == "img"
      || ext == "hdr" || ext == "png" || ext == "jpg" || ext == "jpeg"
      || ext == "tif" || ext == "tiff";
}

} // namespace


template <class TPixel, unsigned int VDim>
ConvertMeshDriver<TPixel, VDim>::ConvertMeshDriver()
    : m_Out(&std::cout), m_Err(&std::cerr)
{
}

template <class TPixel, unsigned int VDim>
const typename ConvertMeshDriver<TPixel, VDim>::Item &
ConvertMeshDriver<TPixel, VDim>::GetVariable(const std::string &name) const
{
  auto it = m_Variables.find(name);
  if(it == m_Variables.end())
    throw ConvertMeshException("undefined variable: " + name);
  return it->second;
}

template <class TPixel, unsigned int VDim>
int ConvertMeshDriver<TPixel, VDim>::ProcessCommandLine(int argc, char *argv[])
{
  try
  {
    if(argc < 2)
    {
      PrintUsage(*m_Out);
      return 0;
    }
    ProcessCommandList(argc - 1, argv + 1);
    return 0;
  }
  catch(ConvertMeshException &e)
  {
    *m_Err << "cmesh: error: " << e.what() << std::endl;
    return 2;
  }
  catch(std::exception &e)
  {
    *m_Err << "cmesh: unexpected error: " << e.what() << std::endl;
    return 3;
  }
}

template <class TPixel, unsigned int VDim>
void ConvertMeshDriver<TPixel, VDim>::ProcessCommandList(int argc, char *argv[])
{
  for(int i = 0; i < argc;)
  {
    const char *tok = argv[i];
    if(tok[0] == '-')
    {
      int consumed = ProcessCommand(argc - i, argv + i);
      // consumed counts arguments *after* argv[0]; step past cmd + args.
      i += 1 + consumed;
    }
    else
    {
      // Bare filename: auto-read as mesh or image based on extension.
      std::string fn = tok;
      if(IsMeshFilename(fn))
      {
        ReadMesh<TPixel, VDim>(this)(fn);
      }
      else if(IsImageFilename(fn))
      {
        ReadImage<TPixel, VDim>(this)(fn);
      }
      else
      {
        throw ConvertMeshException(
          "cannot determine kind from filename extension: " + fn);
      }
      i += 1;
    }
  }
}

template <class TPixel, unsigned int VDim>
int ConvertMeshDriver<TPixel, VDim>::ProcessCommand(int argc, char *argv[])
{
  std::string cmd = argv[0];

  // -------------------- meta --------------------
  if(cmd == "-h" || cmd == "--help" || cmd == "-help")
  {
    PrintUsage(*m_Out);
    return 0;
  }
  if(cmd == "--version" || cmd == "-version")
  {
    PrintVersion(*m_Out);
    return 0;
  }
  if(cmd == "-verbose")
  {
    m_Verbose = true;
    return 0;
  }
  if(cmd == "-no-warn")
  {
    m_WarnOnDataLoss = false;
    return 0;
  }

  // -------------------- sticky backend --------------------
  if(cmd == "-use-vtk") { m_Backend = "vtk"; return 0; }
  if(cmd == "-use-vcg") { m_Backend = "vcg"; return 0; }
  if(cmd == "-use-gpu") { m_Backend = "gpu"; return 0; }
  if(cmd == "-int" || cmd == "-interpolation")
  {
    if(argc < 2) throw ConvertMeshException("-int requires an argument");
    m_Interpolation = ToLower(argv[1]);
    return 1;
  }
  if(cmd == "-discard-data")
  {
    m_DiscardData = true;
    return 0;
  }

  // -------------------- I/O --------------------
  if(cmd == "-o")
  {
    if(argc < 2) throw ConvertMeshException("-o requires a filename");
    std::string fn = argv[1];
    // Write whatever is on top of the stack.
    if(m_Stack.empty()) throw StackAccessException();
    const Item &top = m_Stack.back();
    if(top.IsMesh() || top.IsUGrid())
      WriteMesh<TPixel, VDim>(this)(fn);
    else if(top.IsImage())
      WriteImage<TPixel, VDim>(this)(fn);
    else
      throw ConvertMeshException("stack top has unknown kind");
    return 1;
  }
  if(cmd == "-omesh")
  {
    if(argc < 2) throw ConvertMeshException("-omesh requires a filename");
    WriteMesh<TPixel, VDim>(this)(argv[1]);
    return 1;
  }
  if(cmd == "-oimage")
  {
    if(argc < 2) throw ConvertMeshException("-oimage requires a filename");
    WriteImage<TPixel, VDim>(this)(argv[1]);
    return 1;
  }
  if(cmd == "-push-mesh")
  {
    if(argc < 2) throw ConvertMeshException("-push-mesh requires a filename");
    ReadMesh<TPixel, VDim>(this)(argv[1]);
    return 1;
  }
  if(cmd == "-push-image")
  {
    if(argc < 2) throw ConvertMeshException("-push-image requires a filename");
    ReadImage<TPixel, VDim>(this)(argv[1]);
    return 1;
  }

  // -------------------- stack ops --------------------
  if(cmd == "-pop")   { StackOps<TPixel, VDim>(this).Pop(); return 0; }
  if(cmd == "-dup")   { StackOps<TPixel, VDim>(this).Dup(); return 0; }
  if(cmd == "-swap")  { StackOps<TPixel, VDim>(this).Swap(); return 0; }
  if(cmd == "-clear") { StackOps<TPixel, VDim>(this).Clear(); return 0; }
  if(cmd == "-as")
  {
    if(argc < 2) throw ConvertMeshException("-as requires a variable name");
    StackOps<TPixel, VDim>(this).As(argv[1]);
    return 1;
  }
  if(cmd == "-popas")
  {
    if(argc < 2) throw ConvertMeshException("-popas requires a variable name");
    StackOps<TPixel, VDim>(this).PopAs(argv[1]);
    return 1;
  }
  if(cmd == "-push")
  {
    if(argc < 2) throw ConvertMeshException("-push requires a variable name");
    StackOps<TPixel, VDim>(this).Push(argv[1]);
    return 1;
  }

  throw UnknownCommandException(cmd);
}

template <class TPixel, unsigned int VDim>
void ConvertMeshDriver<TPixel, VDim>::PrintVersion(std::ostream &out) const
{
  out << "cmesh (ConvertMesh) version 0.1.0" << std::endl;
}

template <class TPixel, unsigned int VDim>
void ConvertMeshDriver<TPixel, VDim>::PrintUsage(std::ostream &out) const
{
  out <<
    "cmesh - stack-based mesh and image processing pipeline.\n"
    "\n"
    "Usage: cmesh [ARG|-COMMAND [ARGS...]]...\n"
    "\n"
    "I/O:\n"
    "  FILE                Auto-detect (by extension); push mesh or image onto the stack.\n"
    "  -push-mesh FILE     Read a mesh and push onto the stack.\n"
    "  -push-image FILE    Read an image and push onto the stack.\n"
    "  -o FILE             Write top of stack to FILE (kind inferred from extension).\n"
    "  -omesh FILE         Write top mesh to FILE.\n"
    "  -oimage FILE        Write top image to FILE.\n"
    "\n"
    "Stack ops:\n"
    "  -pop                Discard top of stack.\n"
    "  -dup                Duplicate top of stack.\n"
    "  -swap               Swap top two stack items.\n"
    "  -clear              Empty the stack.\n"
    "  -as NAME            Assign top of stack to variable NAME (keeps on stack).\n"
    "  -popas NAME         Assign top of stack to variable NAME and pop.\n"
    "  -push NAME          Push variable NAME onto the stack.\n"
    "\n"
    "Backend / mode (sticky):\n"
    "  -use-vtk            Prefer VTK-backed implementations (default).\n"
    "  -use-vcg            Prefer VCG-backed implementations.\n"
    "  -use-gpu            Prefer GPU-backed implementations (reserved).\n"
    "  -int MODE           Interpolation mode: linear (default), nn, bspline.\n"
    "  -discard-data       Allow ops that may drop vtkPolyData arrays.\n"
    "  -verbose            Print per-operation progress.\n"
    "  -no-warn            Silence data-loss warnings.\n"
    "\n"
    "Meta:\n"
    "  -h, --help          Print this message.\n"
    "  --version           Print version.\n"
    "\n"
    "Mesh operations will be added incrementally. See the plan document for the roadmap.\n";
}

// --------------------------------------------------------------------------
// Explicit template instantiation
// --------------------------------------------------------------------------
template class ConvertMeshDriver<float, 3>;
