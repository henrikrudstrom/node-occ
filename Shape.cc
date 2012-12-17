#pragma once
#include "Shape.h"
#include "Util.h"
#include "Transformation.h"

#include <map>
#include <vector>

using namespace std;




bool Shape::isNull() 
{
    return shape_.IsNull() ? true : false;
}

bool Shape::isValid()
{
    if (shape_.IsNull())
        return false;
    BRepCheck_Analyzer aChecker(shape_);
    return aChecker.IsValid() ? true : false;
}

const char* Shape::shapeType()
{
 
  switch(shape().ShapeType()) {
    case	TopAbs_COMPOUND:	return "COMPOUND"; break;	
    case	TopAbs_COMPSOLID: 	return "COMPSOLID"; break;	
	case	TopAbs_SOLID: 		return "SOLID"; break;	
	case	TopAbs_SHELL: 		return "SHELL"; break;	
	case	TopAbs_FACE: 		return "FACE"; break;	
	case	TopAbs_WIRE: 		return "WIRE"; break;	
	case	TopAbs_EDGE: 		return "EDGE"; break;	
	case	TopAbs_VERTEX: 		return "VERTEX"; break;	
	case	TopAbs_SHAPE: 		return "SHAPE"; break;	
  }
  assert(0=="invalid case");
  return "???";
}


Handle<Value> Shape::translate(const Arguments& args) 
{
  HandleScope scope;
  Shape* pThis = ObjectWrap::Unwrap<Shape>(args.This());

  gp_Trsf transformation;

  double x=0,y=0,z=0;
  ReadPoint(args[0],&x,&y,&z);
  transformation.SetTranslation(gp_Vec(x,y,z));

  pThis->shape_ = BRepBuilderAPI_Transform(pThis->shape_, transformation).Shape();

  return scope.Close(args.This());

}

Handle<Value> Shape::rotate(const Arguments& args) 
{
  HandleScope scope;
  Shape* pThis = ObjectWrap::Unwrap<Shape>(args.This());

  return scope.Close(args.This());
}

Handle<Value>  Shape::applyTransform(const Arguments& args)
{
  HandleScope scope;
  Shape* pThis = ObjectWrap::Unwrap<Shape>(args.This());

  return scope.Close(args.This());
}

bool operator == (Local<String> str,const char* value)
{
  return str->Equals(String::New(value));
}



//void Shape::ApplyWorkplane(Handle<Object> json) {
//	
//   Local<Object> workplane = json->Get(String::New("workplane"))->ToObject();
//
//   if (!workplane->IsNull()) {
//        
//  
//        Local<Object> workplane_origin = workplane->Get(String::New("origin"))->ToObject();
//        
//        double x = ReadDouble(workplane_origin,"x");
//        double y = ReadDouble(workplane_origin,"y");
//        double z = ReadDouble(workplane_origin,"z");
//
//        Local<Object> workplane_axis = workplane->Get(String::New("axis"))->ToObject();  
//        
//        double u = ReadDouble(workplane_axis,"x");
//        double v = ReadDouble(workplane_axis,"y");
//        double w = ReadDouble(workplane_axis,"z");
//
//        double angle = ReadDouble(workplane,"angle");
//
//        gp_Trsf transformation1 = gp_Trsf();
//        transformation1.SetRotation(gp_Ax1(gp_Pnt(0.0,0.0,0.0), gp_Dir(u,v,w)), angle/180*M_PI);
//        shape_ = BRepBuilderAPI_Transform(shape_, transformation1).Shape();
//
//        gp_Trsf transformation2 = gp_Trsf();
//        transformation2.SetTranslation(gp_Vec(x,y,z));
//        shape_ = BRepBuilderAPI_Transform(shape_, transformation2).Shape();
//    }
//}
//
//void Shape::ApplyReverseWorkplane(Handle<Object> json) {
//	
//   Local<Object> workplane = json->Get(String::New("workplane"))->ToObject();;
//   if (!workplane->IsNull()) {
//
//        Local<Object> workplane_origin = workplane->Get(String::New("origin"))->ToObject();;
//        
//        double x = ReadDouble(workplane_origin,"x");
//        double y = ReadDouble(workplane_origin,"y");
//        double z = ReadDouble(workplane_origin,"z");
//
//        Local<Object> workplane_axis = workplane->Get(String::New("axis"))->ToObject();;       
//        
//        double u = ReadDouble(workplane_axis,"x");
//        double v = ReadDouble(workplane_axis,"y");
//        double w = ReadDouble(workplane_axis,"z");
//
//        double angle = ReadDouble(workplane,"angle");
//        
//        gp_Trsf transformation1 = gp_Trsf();
//        transformation1.SetTranslation(gp_Vec(x,y,z));
//        shape_ = BRepBuilderAPI_Transform(shape_, transformation1).Shape();
//        
//        gp_Trsf transformation2 = gp_Trsf();
//        transformation2.SetRotation(gp_Ax1(gp_Pnt(0.0,0.0,0.0), gp_Dir(u,v,w)), angle/180*M_PI);
//        shape_ = BRepBuilderAPI_Transform(shape_, transformation2).Shape();
//        
//    }
//}




const TopoDS_Shape& Shape::shape()  const
{
    return shape_;
}

void Shape::setShape( const TopoDS_Shape& shape)
{
   shape_ = shape;
}