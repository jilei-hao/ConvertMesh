#ifndef CONVERTMESH_DRIVER_H
#define CONVERTMESH_DRIVER_H

#include "ConvertMeshException.h"
#include "DataItem.h"
#include "DataStack.h"

#include <iostream>
#include <map>
#include <string>

/**
 * Top-level driver for the ConvertMesh CLI. Holds the data stack, sticky
 * state (backend selection, interpolation mode, data-loss policy), parses
 * the command list, and dispatches each command to its adapter.
 *
 * Modelled after c3d's ImageConverter (see
 * itksnap/Submodules/c3d/ConvertImageND.h).
 */
template <class TPixel, unsigned int VDim>
class ConvertMeshDriver
{
public:
  typedef DataItem<TPixel, VDim>       Item;
  typedef DataStack<TPixel, VDim>      StackType;
  typedef typename Item::ImageType     ImageType;
  typedef typename Item::ImagePointer  ImagePointer;
  typedef typename Item::MeshPointer   MeshPointer;
  typedef typename Item::UGridPointer  UGridPointer;

  ConvertMeshDriver();
  ~ConvertMeshDriver() = default;

  // Entry point. Accepts argv including argv[0] (program name).
  int ProcessCommandLine(int argc, char *argv[]);

  // Strips argv[0] and dispatches each token. May throw ConvertMeshException.
  void ProcessCommandList(int argc, char *argv[]);

  // Dispatch a single command. argv[0] is the command token (e.g. "-smooth").
  // Returns the number of *argument* tokens consumed after argv[0].
  int ProcessCommand(int argc, char *argv[]);

  // Named-variable support (c3d-style -as / -popas).
  void SetVariable(const std::string &name, const Item &item) { m_Variables[name] = item; }
  const Item &GetVariable(const std::string &name) const;

  // Redirect output streams (for embedding / testing).
  void RedirectOutput(std::ostream &sout, std::ostream &serr)
  {
    m_Out = &sout; m_Err = &serr;
  }
  std::ostream &Out() { return *m_Out; }
  std::ostream &Err() { return *m_Err; }
  std::ostream &Verbose() { return m_Verbose ? *m_Out : m_Null; }

  // Help text.
  void PrintUsage(std::ostream &out) const;
  void PrintVersion(std::ostream &out) const;

  // -------------------------------------------------------------------------
  // Public state touched by adapters. These are exposed deliberately (c3d
  // does the same via friend class) so adapters can read/write the stack.
  // -------------------------------------------------------------------------
  StackType m_Stack;
  std::map<std::string, Item> m_Variables;

  // Sticky state
  std::string m_Backend      = "vtk";  // -use-vtk / -use-vcg / -use-gpu
  std::string m_Interpolation = "linear"; // -int linear|nn|bspline
  bool        m_DiscardData  = false;  // -discard-data (latched)
  bool        m_Verbose      = false;  // -verbose
  bool        m_WarnOnDataLoss = true; // -no-warn to silence

private:
  std::ostream *m_Out;
  std::ostream *m_Err;
  // Verbose sink when m_Verbose == false: a reusable no-op stream that
  // discards input. We rebuild it lazily.
  class NullStream : public std::ostream
  {
  public:
    NullStream() : std::ostream(&m_Buf) {}
  private:
    class NullBuf : public std::streambuf
    {
    protected:
      int overflow(int c) override { return c; }
    } m_Buf;
  };
  NullStream m_Null;
};

#endif
