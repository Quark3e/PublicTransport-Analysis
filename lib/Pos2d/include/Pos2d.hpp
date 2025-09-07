#pragma once
#ifndef _VAR__Pos2d
#define _VAR__Pos2d

#include <cmath>
#include <cstdlib>

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>


#ifndef M_PI
#define M_PI  3.1415926535
#endif


#ifndef toDEGREES
#define toDEGREES(RAD) ((RAD)*180/M_PI)
#endif
#ifndef toRADIANS
#define toRADIANS(DEG) ((DEG)*M_PI/180.0)
#endif


template<typename _varType>
struct Pos2d {
    _varType x;
    _varType y;
    Pos2d() {}
    // Pos2d(_varType *_arrPtr): x(_arrPtr[0]), y(_arrPtr[1]) {}
    Pos2d(const Pos2d &_copy) {
        this->x = _copy.x;
        this->y = _copy.y;
    }
    Pos2d(_varType _x, _varType _y): x(_x), y(_y) {}
    _varType &operator[](size_t _i) {
        switch (_i) {
            case 0: return this->x;
            case 1: return this->y;
        }
        throw std::out_of_range("size_t index value is out of range.");
        return x;
    }
    _varType operator[](size_t _i) const {
        switch (_i) {
            case 0: return this->x;
            case 1: return this->y;
        }
        throw std::out_of_range("size_t index value is out of range.");
        return x;
    }

    Pos2d &operator=(const Pos2d<_varType>& m) {
        this->x = m.x;
        this->y = m.y;
        return *this;
    }
    bool operator<(Pos2d const& m)  { return ((x-m.x)+(y-m.y))<0; }
    bool operator>(Pos2d const& m)  { return ((x-m.x)+(y-m.y))>0; }
    int equalCompare_round_prec = 6;
    bool operator==(Pos2d const& m) {
        return (
            (roundf(m.x*pow(10, equalCompare_round_prec))/pow(10, equalCompare_round_prec)==roundf(this->x*pow(10, equalCompare_round_prec))/pow(10, equalCompare_round_prec)) &&
            (roundf(m.y*pow(10, equalCompare_round_prec))/pow(10, equalCompare_round_prec)==roundf(this->y*pow(10, equalCompare_round_prec))/pow(10, equalCompare_round_prec))
        );
        // return (x==m.x && y==m.y);
    }
    bool operator!=(Pos2d const& m) { return !(x==m.x && y==m.y); }

    operator std::string() {
        std::stringstream ss;
        ss << std::setw(_printWidth) << std::setprecision(_printPrecision);
        ss << "("<<x<<","<<y<<")";
        return ss.str();
    }

    /**
     * @brief Check whether a Pos2d value is within the region given by min-max corners of a Bounding Box.
     * 
     * @note The function relies on the user to properly apply the min/max coordinates. This member function does
     * not do internal checking.
     * 
     * @param corner_min corner coordinate with minimum values.
     * @param corner_max corner coordinate with maximum values.
     * @param _includeBB whether to include the Bounding Box borner region (ex: `>=`) or not (`>`)
     * @return true is within region.
     * @return false is not within region.
     */
    bool inRegion(Pos2d corner_min, Pos2d corner_max, bool _includeBB=true) {
        bool inReg = false;
        if(_includeBB) {
            inReg = ((x>=corner_min.x && y>=corner_min.y) && (x<=corner_max.x && y<=corner_max.y));
        }
        else {
            inReg = ((x> corner_min.x && y> corner_min.y) && (x< corner_max.x && y< corner_max.y));
        }

        return inReg;
    }

    Pos2d& operator+=(Pos2d const& m) {
        x+=m.x;
        y+=m.y;
        return *this;
    }
    Pos2d& operator-=(Pos2d const& m) {
        x-=m.x;
        y-=m.y;
        return *this;
    }
    Pos2d& operator*=(Pos2d const& m) {
        x*=m.x;
        y*=m.y;
        return *this;
    }
    Pos2d& operator/=(Pos2d const& m) {
        x/=m.x;
        y/=m.y;
        return *this;
    }
    
    Pos2d operator+(Pos2d const& m) {
        return Pos2d(x+m.x, y+m.y);
    }
    Pos2d operator-(Pos2d const& m) {
        return Pos2d(x-m.x, y-m.y);
    }
    Pos2d operator*(Pos2d const& m) {
        return Pos2d(x*m.x, y*m.y);
    }
    Pos2d operator/(Pos2d const& m) {
        return Pos2d(x/m.x, y/m.y);
    }
    
    Pos2d operator+(_varType var) {
        return Pos2d(x+var, y+var);
    }
    Pos2d operator-(_varType var) {
        return Pos2d(x-var, y-var);
    }
    Pos2d operator*(_varType var) {
        return Pos2d(x*var, y*var);
    }
    Pos2d operator/(_varType var) {
        return Pos2d(x/var, y/var);
    }
    
    
    Pos2d& operator+=(_varType var) {
        x+=var;
        y+=var;
        return *this;
    }
    Pos2d& operator-=(_varType var) {
        x-=var;
        y-=var;
        return *this;
    }
    Pos2d& operator*=(_varType var) {
        x*=var;
        y*=var;
        return *this;
    }
    Pos2d& operator/=(_varType var) {
        x/=var;
        y/=var;
        return *this;
    }
    

    Pos2d& rotate(Pos2d pivotPoint, double rotateAngle) {
        Pos2d delta(x-pivotPoint.x, y-pivotPoint.y);

        x = pivotPoint.x + delta.x*cos(toRADIANS(rotateAngle)) - delta.y*sin(toRADIANS(rotateAngle));
        y = pivotPoint.y + delta.x*sin(toRADIANS(rotateAngle)) + delta.y*cos(toRADIANS(rotateAngle));

        return *this;
    }
    
    
    template<typename _lambda>
    Pos2d getModify(_lambda modificationFunc) {
        return Pos2d(modificationFunc(x), modificationFunc(y));
    }

    template<typename _castType>
    Pos2d<_castType> cast() {
        return Pos2d<_castType>(_castType(x), _castType(y));
    }
    template<typename _castType, typename _lambda>
    Pos2d<_castType> cast(_lambda modificationFunc) {
        return Pos2d<_castType>(modificationFunc(x), modificationFunc(y));
    }

    Pos2d getAbs() {
        return Pos2d(
            (x>0? x : x*(-1)),
            (y>0? y : y*(-1))
        );
    }
    _varType getHypotenuse() {
        return sqrt(pow(x, 2)+pow(y, 2));
    }
    _varType getHypotenuse() const {
        return sqrt(pow(x, 2)+pow(y, 2));
    }

    _varType getDelta(bool reverse=false) {
        if(reverse) return x-y;
        return y-x;
    }

    Pos2d getRounded(int decimals) {
        return Pos2d(
            roundf(this->x*pow(10, decimals)) / pow(10, decimals),
            roundf(this->y*pow(10, decimals)) / pow(10, decimals)
        );
    }

    Pos2d getRotated(Pos2d pivotPoint, double rotateAngle) {
        Pos2d delta(x-pivotPoint.x, y-pivotPoint.y);
        return Pos2d(
            pivotPoint.x + delta.x*cos(toRADIANS(rotateAngle)) - delta.y*sin(toRADIANS(rotateAngle)),
            pivotPoint.y + delta.x*sin(toRADIANS(rotateAngle)) + delta.y*cos(toRADIANS(rotateAngle))
        );
    }

    double getTheta(Pos2d pivotPoint, bool nonNegative=true) {
        Pos2d delta(x-pivotPoint.x, y-pivotPoint.y);
        
        if(delta.y>0) {
            return acos(delta.x/delta.getHypotenuse());
        }
        else {
            if(!nonNegative) return -acos(delta.x/delta.getHypotenuse());
            else return 2.0*M_PI-acos(delta.x/delta.getHypotenuse());
        }
    }

    int _printPrecision = 2;
    int _printWidth     = 0;
    friend auto operator<<(std::ostream &os, Pos2d const& m) -> std::ostream& {
        os << std::setw(m._printWidth) << std::setprecision(m._printPrecision);
        os << "("<<m.x<<","<<m.y<<")";
        return os;
    }
};

#endif //_VAR__Pos2d