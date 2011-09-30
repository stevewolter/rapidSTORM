#ifndef LOCPREC_BIPLANE_MOTIONMODELS_H
#define LOCPREC_BIPLANE_MOTIONMODELS_H

#include "impl.h"
#include <simparm/Object.hh>

namespace locprec {
namespace biplane_alignment {

struct NoMotion : public simparm::Object, public MotionModel {
    NoMotion() : simparm::Object("NoAlignment", "No alignment") {}
    NoMotion* clone() const { return new NoMotion(*this); }
    simparm::Node& getNode() { return *this; }
    static inline void initialize_field_jacobian(FieldJacobian&) { throw IsIdentity(); }
    static inline void set_field_jacobian(FieldJacobian&, int, int) { throw IsIdentity(); }
    Motion get_motion( const Eigen::Matrix<double, Eigen::Dynamic, 1>& ) const { throw IsIdentity(); }
    bool is_identity() const { return true; }
    int parameter_count() const { throw IsIdentity(); }
};

struct Translation : public simparm::Object, public MotionModel {
    Translation() : simparm::Object("Translation", "Translation") {}
    Translation* clone() const { return new Translation(); }
    simparm::Node& getNode() { return *this; }
    static void initialize_field_jacobian(FieldJacobian& t) { 
        t = Eigen::Matrix2d::Identity(); 
    }
    static void set_field_jacobian(FieldJacobian&, int, int) {}
    Motion get_motion( const Eigen::Matrix<double, Eigen::Dynamic, 1>& variables ) const { 
        Motion m = Motion::Identity(); 
        m.block<2,1>(0,2) = variables; 
        return m; 
    }
    int parameter_count() const { return 2; }
};

struct ScaledTranslation : public simparm::Object, public MotionModel {
    ScaledTranslation() : simparm::Object("ScaledTranslation", "ScaledTranslation") {}
    ScaledTranslation* clone() const { return new ScaledTranslation(); }
    simparm::Node& getNode() { return *this; }
    static void initialize_field_jacobian(FieldJacobian& t) { 
        t = Eigen::Matrix<double,2,4>::Zero();
        t.block<2,2>(0,0) = Eigen::Matrix2d::Identity(); 
    }
    static void set_field_jacobian(FieldJacobian& t, int x, int y) {
        t(0,2) = x;
        t(1,3) = y;
    }
    Motion get_motion( const Eigen::Matrix<double, Eigen::Dynamic, 1>& variables ) const { 
        Motion m = Motion::Identity(); 
        m.block<2,1>(0,2) = variables.start<2>(); 
        m(0,0) = 1+variables[2];
        m(1,1) = 1+variables[3];
        return m; 
    }
    int parameter_count() const { return 4; }
};

struct Euclidean : public simparm::Object, public MotionModel {
    Euclidean() : simparm::Object("Euclidean", "Euclidean") {}
    Euclidean* clone() const { return new Euclidean(); }
    simparm::Node& getNode() { return *this; }
    static void initialize_field_jacobian(FieldJacobian& t) { throw IsIdentity(); }
    static void set_field_jacobian(FieldJacobian&, int, int) {}
    Motion get_motion( const Eigen::Matrix<double, Eigen::Dynamic, 1>& variables ) const { 
        Motion m = Motion::Identity(); 
        m.block<2,1>(0,2) = variables.start<2>(); 
        m(0,0) = m(1,1) = cos( variables[2] );
        m(0,1) = -sin( variables[2] );
        m(1,0) = - m(0,1);
        return m; 
    }
    int parameter_count() const { return 3; }
};

struct Similarity : public simparm::Object, public MotionModel {
    Similarity() : simparm::Object("Similarity", "Similarity") {}
    Similarity* clone() const { return new Similarity(); }
    simparm::Node& getNode() { return *this; }
    static void initialize_field_jacobian(FieldJacobian& t) { 
        t = Eigen::Matrix<double,2,4>::Zero();
        t.block<2,2>(0,0) = Eigen::Matrix2d::Identity();
    }
    static void set_field_jacobian(FieldJacobian& t, int x, int y) { 
        t(0,2) = t(1,3) = x;
        t(1,2) = -y; t(0,3) = y;
    }
    Motion get_motion( const Eigen::Matrix<double, Eigen::Dynamic, 1>& variables ) const { 
        Motion m = Motion::Identity(); 
        m.block<2,1>(0,2) = variables.start<2>(); 
        m(0,0) = m(1,1) = 1 + variables[2];
        m(0,1) = - variables[3];
        m(1,0) = - m(0,1);
        return m; 
    }
    int parameter_count() const { return 4; }
};

struct Affine : public simparm::Object, public MotionModel {
    Affine() : simparm::Object("Affine", "Affine") {}
    Affine* clone() const { return new Affine(); }
    simparm::Node& getNode() { return *this; }
    static void initialize_field_jacobian(FieldJacobian& t) { 
        t = Eigen::Matrix<double,2,6>::Zero();
        t.block<2,2>(0,0) = Eigen::Matrix2d::Identity();
    }
    static void set_field_jacobian(FieldJacobian& t, int x, int y) { 
        t(0,2) = t(1,4) = x;
        t(0,3) = t(1,5) = y;
    }
    Motion get_motion( const Eigen::Matrix<double, Eigen::Dynamic, 1>& variables ) const { 
        Motion m = Motion::Identity(); 
        m.block<2,1>(0,2) = variables.start<2>(); 
        m(0,0) = 1 + variables[2];
        m(1,0) = variables[3];
        m(0,1) = variables[4];
        m(1,1) = 1 + variables[5];
        return m; 
    }
    int parameter_count() const { return 6; }
};

}
}

#endif
