/**
* @file Pose3D.h
* Contains class Pose3D
* @author <a href="mailto:martin.kallnik@gmx.de">Martin Kallnik</a>
* @author Max Risler
*/

#ifndef Pose3D_H
#define Pose3D_H

#include <iostream>
#include "RotationMatrix.h"
#include "Pose2D.h"

/** representation for 3D Transformation (Location + Orientation)*/
class Pose3D
{
public:

  /** rotation as a RotationMatrix*/
  RotationMatrix rotation;

  /** translation as a Vector3*/
  Vector3<float> translation;

  /** constructor*/
  Pose3D() {}

  /** constructor from rotation and translation
   * \param rot Rotation
   * \param trans Translation
   */
  Pose3D(const RotationMatrix& rot, const Vector3<float>& trans): rotation(rot), translation(trans){}

  /** constructor from rotation
   * \param rot Rotation
   */
  Pose3D(const RotationMatrix& rot): rotation(rot) {}

  /** constructor from translation
   * \param trans Translation
   */
  Pose3D(const Vector3<float>& trans): translation(trans) {}

  /** constructor from three translation values
   * \param x translation x component
   * \param y translation y component
   * \param z translation z component
   */
  Pose3D(const float x, const float y, const float z) : translation(x,y,z) {}

  /** constructor from Pose2D */
  Pose3D(const Pose2D p) {
    translation.x = p.translation.x;
    translation.y = p.translation.y;
    translation.z = 0;
    rotation.rotateZ(-p.rotation);
  }

  /** Assignment operator
  *\param other The other Pose3D that is assigned to this one
  *\return A reference to this object after the assignment.
  */
  Pose3D& operator=(const Pose3D& other)
  {
    rotation=other.rotation;
    translation=other.translation;
    
    return *this;
  }

  /** Copy constructor
  *\param other The other vector that is copied to this one
  */
  Pose3D(const Pose3D& other) {*this = other;}

  /** Multiplication with Point
  *\param point (Vector3&lt;float&gt;)
  */
  Vector3<float> operator*(const Vector3<float>& point) const
  {
    return rotation * point + translation;
  }

  /** Comparison of another vector with this one.
  *\param other The other vector that will be compared to this one
  *\return Whether the two vectors are equal.
  */
  bool operator==(const Pose3D& other) const
  {
    return ((translation==other.translation)&&(rotation==other.rotation));
  }

  /** Comparison of another vector with this one.
  *\param other The other vector that will be compared to this one
  *\return Whether the two vectors are unequal.
  */
  bool operator!=(const Pose3D& other) const
    {return !(*this == other);}

  /**Concatenation of this pose with another pose
  *\param other The other pose that will be concatenated to this one.
  *\return A reference to this pose after concatenation
  */
  Pose3D& conc(const Pose3D& other)
  {
    translation = *this * other.translation;
    rotation *= other.rotation;
    return *this;
  }

  Pose3D relativeTo(const Pose3D& other)
  {
    Pose3D result;
    result.rotation = other.rotation.invert() * this->rotation;
    result.translation = other.rotation.invert() * (-other.translation + this->translation);
    return result;
  }
  
  /** Calculates the inverse transformation from the current pose
  * @return The inverse transformation pose.
  */
  Pose3D invert() const
  {
    Pose3D result;
    result.rotation = this->rotation.invert();
    result.translation = result.rotation * (Vector3<float>(0, 0, 0) - this->translation);
    return result;
  }

  /**Translate this pose by a translation vector
  *\param trans Vector to translate with
  *\return A reference to this pose after translation
  */
  Pose3D& translate(const Vector3<float>& trans)
  {
    translation = *this * trans;
    return *this;
  }

  /**Translate this pose by a translation vector
  *\param x x component of vector to translate with
  *\param y y component of vector to translate with
  *\param z z component of vector to translate with
  *\return A reference to this pose after translation
  */
  Pose3D& translate(const float x, const float y, const float z)
  {
    Vector3<float> vec(x,y,z);
    return translate(vec);
  }

  /**Rotate this pose by a rotation
  *\param rot Rotationmatrix to rotate with
  *\return A reference to this pose after rotation
  */
  Pose3D& rotate(const RotationMatrix& rot)
  {
    rotation *= rot;
    return *this;
  }

  /**Rotate this pose around its x-axis
  *\param angle angle to rotate with
  *\return A reference to this pose after rotation
  */
  Pose3D& rotateX(const float angle)
  {
    rotation.rotateX(angle);
    return *this;
  }
  
  /**Rotate this pose around its y-axis
  *\param angle angle to rotate with
  *\return A reference to this pose after rotation
  */
  Pose3D& rotateY(const float angle)
  {
    rotation.rotateY(angle);
    return *this;
  }
  
  /**Rotate this pose around its z-axis
  *\param angle angle to rotate with
  *\return A reference to this pose after rotation
  */
  Pose3D& rotateZ(const float angle)
  {
    rotation.rotateZ(angle);
    return *this;
  }

  Pose3D relativeToGlobal(const Pose2D &origin){
    Pose3D retval(*this);
    RotationMatrix rot;
    rot.rotateZ(origin.rotation);
    retval.translation = rot * retval.translation;
    retval.translation.x += origin.translation.x;
    retval.translation.y += origin.translation.y;
    retval.rotation.rotateZ(origin.rotation);
    return retval;
  }

  Pose3D relativeToGlobal(const Pose3D &origin){
    Pose3D retval(*this);
    retval.translation = origin.rotation * retval.translation;
    retval.translation += origin.translation;
    retval.rotation = origin.rotation * retval.rotation;
    return retval;
  }

  Pose3D globalToRelative(const Pose2D &origin){
    Pose3D retval(*this);
    retval.translation.x -= origin.translation.x;
    retval.translation.y -= origin.translation.y;
    RotationMatrix rot;
    rot.rotateZ(-origin.rotation);
    retval.translation = rot * retval.translation;
    retval.rotation.rotateZ(-origin.rotation);
    return retval;
  }

  Pose3D globalToRelative(const Pose3D &origin){
    Pose2D origin_2d(0,0,0);
    origin_2d.translation.x = origin.translation.x;
    origin_2d.translation.y = origin.translation.y;
    origin_2d.rotation = origin.rotation.getZAngle();
    return this->globalToRelative(origin_2d);
  }
  friend std::ostream& operator<<(std::ostream& out, const Pose3D& pose) {
    out << "translation: " << pose.translation << "\n";
    out << "rotation:\n";
    out << pose.rotation << "\n";
    return out;
  }
  
  float* data() {
    return &rotation.c[0][0];
  }

  const float* data() const {
    return &rotation.c[0][0];
  }

  inline int size() const { return 12; }
};

#endif // Pose3D_H
