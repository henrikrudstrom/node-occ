#include "Face.h"
#include "Wire.h"
#include "Edge.h"
#include "Util.h"
#include "Mesh.h"

Face::~Face() {
	m_cacheMesh.Reset();
};

const TopoDS_Shape&  Face::shape() const
{
  return face();
}

void Face::setShape( const TopoDS_Shape& shape)
{
  m_face = TopoDS::Face(shape);
}

int Face::numWires()
{
  if(shape().IsNull()) return 0;
  TopTools_IndexedMapOfShape anIndices;
  TopExp::MapShapes(shape(), TopAbs_WIRE, anIndices);
  return anIndices.Extent();
}

bool Face::fixShape()
{
  return true;
}

double Face::area()
{
  GProp_GProps prop;
  BRepGProp::SurfaceProperties(shape(), prop);
  return prop.Mass();
}

bool Face::hasMesh()
{
  TopLoc_Location loc;
  occHandle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(this->face(), loc);
  return triangulation.IsNull()?false:true;
}

std::vector<double> Face::inertia()
{
  std::vector<double> ret;
  GProp_GProps prop;
  BRepGProp::SurfaceProperties(this->shape(), prop);
  gp_Mat mat = prop.MatrixOfInertia();
  ret.push_back(mat(1,1)); // Ixx
  ret.push_back(mat(2,2)); // Iyy
  ret.push_back(mat(3,3)); // Izz
  ret.push_back(mat(1,2)); // Ixy
  ret.push_back(mat(1,3)); // Ixz
  ret.push_back(mat(2,3)); // Iyz
  return ret;
}


const gp_XYZ Face::centreOfMass() const
{

  GProp_GProps prop;
  BRepGProp::SurfaceProperties(this->shape(), prop);
  gp_Pnt cg = prop.CentreOfMass();

  return cg.Coord();
}

bool Face::isPlanar()
{

  Handle_Geom_Surface surf = BRep_Tool::Surface(this->m_face);
  GeomLib_IsPlanarSurface tool(surf);
  return tool.IsPlanar() ? true : false;
}

Nan::Persistent<v8::FunctionTemplate> Face::_template;

bool Face::buildFace(std::vector<Wire*>& wires)
{
  if (wires.size()==0) return false;

  // checling that all wires are closed
  for (size_t i = 0; i < wires.size(); i++) {
    if (!wires[i]->isClosed()) {
      Nan::ThrowError("Some of the wires are not closed");
      return false;
    }
  }

  try {
    const TopoDS_Wire& outerwire = wires[0]->wire();

    BRepBuilderAPI_MakeFace MF(outerwire);

    // add optional holes
    for (unsigned i = 1; i < wires.size(); i++) {
      const TopoDS_Wire& wire = wires[i]->wire();

      if (wire.Orientation() != outerwire.Orientation()) {
        MF.Add(TopoDS::Wire(wire.Reversed()));
      } else {
        MF.Add(wire);
      }
    }
    this->setShape(MF.Shape());

    // possible fix shape
    if (!this->fixShape()) {
      StdFail_NotDone::Raise("Shapes not valid");
    }

  }
  CATCH_AND_RETHROW("Failed to create a face");
  return true;
}

NAN_METHOD(Face::New)
{
  Face* obj = new Face();
  obj->Wrap(info.This());
  obj->InitNew(info);

  std::vector<Wire*> wires;
  extractArgumentList(info,wires);
  obj->buildFace(wires);

  info.GetReturnValue().Set(info.This());

}

v8::Local<v8::Object> Face::Clone() const
{
  Face* obj = new Face();
  v8::Local<v8::Object> instance = Nan::New(_template)->GetFunction()->NewInstance();
  obj->Wrap(instance);
  obj->setShape(this->shape());
  return instance;
}

v8::Handle<v8::Object> Face::NewInstance(const TopoDS_Face& face)
{
  Face* obj = new Face();
  v8::Local<v8::Object> instance = Nan::New(_template)->GetFunction()->NewInstance();
  obj->Wrap(instance);
  obj->setShape(face);
  return instance;
}

NAN_PROPERTY_GETTER(Face::_mesh)
{
  // NanScope();
  if (info.This().IsEmpty()) {
	return info.GetReturnValue().SetUndefined();
  }
  if (info.This()->InternalFieldCount() == 0 ) {
	return info.GetReturnValue().SetUndefined();
  }

  Face* pThis = ObjectWrap::Unwrap<Face>(info.This());
  if (pThis->m_cacheMesh.IsEmpty()) {
	  pThis->m_cacheMesh.Reset(pThis->createMesh(0.5, 20 * 3.14159 / 180.0, true));
  }
  info.GetReturnValue().Set(Nan::New(pThis->m_cacheMesh));
}

v8::Handle<v8::Object> Face::createMesh(double factor, double angle, bool qualityNormals)
{
  Nan::EscapableHandleScope scope;
  const unsigned argc = 0;
  v8::Handle<v8::Value> argv[1] = {  };
  v8::Local<v8::Object> theMesh = Nan::New(Mesh::_template)->GetFunction()->NewInstance(argc, argv);

  Mesh *mesh =  Mesh::Unwrap<Mesh>(theMesh);

  const TopoDS_Shape& shape = this->shape();

  try {
    // this code assume that the triangulation has been created
    // on the parent object
    mesh->extractFaceMesh(this->face(), qualityNormals);

  } CATCH_AND_RETHROW("Failed to mesh solid ");
  mesh->optimize();

  return scope.Escape(theMesh);
  //xx return NanEscapeScope(th;eMesh);

}


void Face::InitNew(_NAN_METHOD_ARGS)
{
  Base::InitNew(info);
  REXPOSE_READ_ONLY_PROPERTY_DOUBLE(Face,area);

  REXPOSE_READ_ONLY_PROPERTY_INTEGER(Face,numWires);
  REXPOSE_READ_ONLY_PROPERTY_DOUBLE(Face,area);
  REXPOSE_READ_ONLY_PROPERTY_BOOLEAN(Face,isPlanar);
  REXPOSE_READ_ONLY_PROPERTY_BOOLEAN(Face,hasMesh);
}
void Face::Init(v8::Handle<v8::Object> target)
{
  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(Face::New);
  tpl->SetClassName(Nan::New("Face").ToLocalChecked());

  // object has one internal filed ( the C++ object)
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  _template.Reset(tpl);

  // Prototype
  v8::Local<v8::ObjectTemplate> proto = tpl->PrototypeTemplate();

  Base::InitProto(proto);

  EXPOSE_READ_ONLY_PROPERTY_INTEGER(Face,numWires);
  EXPOSE_READ_ONLY_PROPERTY_DOUBLE(Face,area);
  EXPOSE_READ_ONLY_PROPERTY_BOOLEAN(Face,isPlanar);
  EXPOSE_READ_ONLY_PROPERTY_BOOLEAN(Face,hasMesh);
  EXPOSE_READ_ONLY_PROPERTY(_mesh,mesh);
  EXPOSE_TEAROFF(Face,centreOfMass);
  target->Set(Nan::New("Face").ToLocalChecked(), tpl->GetFunction());
}
