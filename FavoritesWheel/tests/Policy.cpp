#include "../src/Policy.h"
#include <cstdlib>
#include <iostream>
#include <limits>
#define CHECK(x) do { if (!(x)) { std::cerr << "failed line " << __LINE__ << "\n"; std::exit(1); } } while(false)
int main() {
 using namespace WheelPolicy;
 CHECK(Hit(0,0,70,8)==-1); CHECK(Hit(0,-200,70,4)==0);
 CHECK(Hit(200,0,70,4)==1); CHECK(Hit(0,200,70,4)==2); CHECK(Hit(-200,0,70,4)==3);
 CHECK(Hit(100,0,70,0)==-1); CHECK(Hit(std::numeric_limits<float>::quiet_NaN(),0,70,4)==-1);
 CHECK(Wrap(-1,3)==2); CHECK(Wrap(3,3)==0); CHECK(Pages(0)==1); CHECK(Pages(10)==1); CHECK(Pages(11)==2); CHECK(Pages(103)==11);
 for (int n=1;n<=Slots;++n) for(int i=0;i<n;++i) {
  float a=2.f*std::numbers::pi_v<float>*i/n-std::numbers::pi_v<float>/2.f;
  CHECK(Hit(200*std::cos(a),200*std::sin(a),70,n)==i);
 }
 std::cout << "PASS: radial selection, center cancellation, wraparound and overflow paging\n";
}
