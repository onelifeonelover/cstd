#include <nana/gui/wvl.hpp> 
#include <nana/gui/widgets/button.hpp> 
#include <nana/gui/place.hpp> 
#include <iostream> 


struct data { string url; }; 

class uiface 
{ public: 
    virtual ~uiface() = 0; 
    virtual void create_ui_element(const data&) = 0; 
}; 

uiface::~uiface(){} 

class framework 
{ 
    uiface            *uiface_{}; 
    vector<data>  cont_; 

public: 
    framework()
    { 
        data dat; 
        dat.url = "http://nanapro.org/"; 
        cont_.push_back(dat); 

        dat.url = "nanaproject.wordpress.com"; 
        cont_.push_back(dat); 
    } 
    void set(uiface * uif) { uiface_ = uif; } 
    void work() 
    { 
        if(uiface_) 
          for(auto & ui_el : cont_ ) 
              uiface_->create_ui_element(ui_el); 
    } 
}; 


namespace ui 
{ 
    using namespace nana; 

    class bar : public form, public uiface 
    { 
        struct command 
        {  data dat; 
           command(const data& d): dat(d) {} 
            void operator()() { cout<<"open "<<dat.url<<endl; } 
        }; 

        place  plc_{*this}; 
        vector<shared_ptr<button>> ui_el_; 

     public: 
        bar() {  plc_.div("<vertical abc gap=3>");} 

      private: 
               //Now we implement the virtual function declared by uiface 
        void create_ui_element(const data& dat)
        { 
            auto p= make_shared<button>(*this) ; 

                          //Make the click event 
            p->events().click(  command(dat) ); 
            p->caption(dat.url) ; 

            plc_["abc"]<< *p; 
            plc_.collocate();

            ui_el_.push_back(p); 
        } 
    }; 
} 


int main() 
{ 
	ui::bar bar; 
	bar.show(); 
	framework fw; 
	fw.set(&bar); 
	fw.work(); 
	exec(); 
} 