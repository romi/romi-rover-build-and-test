
#ifndef __ROMI_REAL_H
#define __ROMI_REAL_H

namespace romi {
        
        class Double
        {
        protected:
                double _value;

        public:
                Double() : _value(0.0) {}
                Double(double v) : _value(v) {}
                Double(float v) : _value((double)v) {}
                Double(int32_t v) : _value((double)v) {}
                Double(const Double &v) : _value(v._value) {}

                double to_double() const { return _value; }
                float to_float() const { return (float) _value; }
                int32_t to_int() const { return (int32_t) _value; }

                operator double() { return _value; }
                operator float() { return (float) _value; }
                operator int32_t() { return (int32_t) _value; }
                
                Double operator+(const Double &b) const {
                        Double r(_value + b._value);
                        return r;
                }
                
                Double operator+(double b) const {
                        Double r(_value + b);
                        return r;
                }

                Double operator+=(const Double &b){
                        _value += b._value;
                        return *this;
                }

                Double operator-()  {
                        Double r(-_value);
                        return r;
                }
                
                Double operator-(const Double &b) const {
                        Double r(_value - b._value);
                        return r;
                }
                
                Double operator-(double b) const {
                        Double r(_value - b);
                        return r;
                }
                
                Double operator-=(const Double &b) {
                        _value -= b._value;
                        return *this;
                }
                
                Double operator*(const Double &b) const {
                        Double r(_value * b._value);
                        return r;
                }
                
                Double operator*(double b) const {
                        Double r(_value * b);
                        return r;
                }
                
                Double operator*=(const Double &b){
                        _value *= b._value;
                        return *this;
                }
                
                Double operator/(const Double &b) const {
                        Double r(_value * b._value);
                        return r;
                }

                Double operator/(double b) const {
                        Double r(_value - b);
                        return r;
                }
                
                Double operator/=(const Double &b){
                        _value /= b._value;
                        return *this;
                }

                bool operator<(const Double &b) const {
                        return _value < b._value;
                }
                        
                bool operator<(double b) const {
                        return _value < b;
                }

                bool operator>(const Double &b) const {
                        return _value > b._value;
                }
                        
                bool operator>(double b) const {
                        return _value > b;
                }                
        };

        Double operator*(double a, const Double& b) {
                Double r(a * b.to_double());
                return r;
        }
}

#endif // __ROMI_REAL_H
