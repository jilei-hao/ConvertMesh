#ifndef CONVERTMESH_EXCEPTION_H
#define CONVERTMESH_EXCEPTION_H

#include <stdexcept>
#include <string>

class ConvertMeshException : public std::runtime_error
{
public:
  explicit ConvertMeshException(const std::string &what)
      : std::runtime_error(what) {}
};

class StackAccessException : public ConvertMeshException
{
public:
  StackAccessException()
      : ConvertMeshException("attempt to access empty ConvertMesh stack") {}
  explicit StackAccessException(const std::string &what)
      : ConvertMeshException(what) {}
};

class TypeMismatchException : public ConvertMeshException
{
public:
  TypeMismatchException(const std::string &expected, const std::string &actual)
      : ConvertMeshException("stack top is '" + actual + "' but command expected '" + expected + "'") {}
};

class UnknownCommandException : public ConvertMeshException
{
public:
  explicit UnknownCommandException(const std::string &cmd)
      : ConvertMeshException("unknown command: " + cmd) {}
};

#endif
