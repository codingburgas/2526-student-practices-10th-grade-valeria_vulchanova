#include "theme.h"
#include <cmath>
#include <algorithm>
#include <sstream>

// ── Colours ───────────────────────────────────────────────────
const Color C_BG     ={5,5,10,255};    const Color C_PANEL  ={12,11,20,255};
const Color C_GOLD   ={212,163,57,255};const Color C_GOLD2  ={255,210,100,255};
const Color C_GOLDDIM={100,76,22,255}; const Color C_WHITE  ={235,232,225,255};
const Color C_GREY   ={95,92,100,255}; const Color C_GREYLT ={155,152,162,255};
const Color C_INPUTBG={18,17,28,255};  const Color C_RED    ={210,60,60,255};
const Color C_GREEN  ={60,200,100,255};const Color C_ACCENT ={80,160,220,255};

// ── Fonts ─────────────────────────────────────────────────────
Font fTitle,fUI,fUIBold;
void LoadFonts(){
    fTitle = FileExists("Cinzel-Bold.ttf")   ? LoadFontEx("Cinzel-Bold.ttf",80,nullptr,0)   : GetFontDefault();
    fUI    = FileExists("Raleway-Regular.ttf")? LoadFontEx("Raleway-Regular.ttf",32,nullptr,0): GetFontDefault();
    fUIBold= FileExists("Raleway-Bold.ttf")  ? LoadFontEx("Raleway-Bold.ttf",32,nullptr,0)   : GetFontDefault();
    SetTextureFilter(fTitle.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(fUI.texture,    TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(fUIBold.texture,TEXTURE_FILTER_BILINEAR);
}
void UnloadFonts(){UnloadFont(fTitle);UnloadFont(fUI);UnloadFont(fUIBold);}

// ── Colour helpers ────────────────────────────────────────────
Color CA(Color c,float a){c.a=(unsigned char)(std::clamp(a,0.f,1.f)*255.f);return c;}
Color LerpC(Color a,Color b,float t){return{(unsigned char)(a.r+(b.r-a.r)*t),(unsigned char)(a.g+(b.g-a.g)*t),(unsigned char)(a.b+(b.b-a.b)*t),(unsigned char)(a.a+(b.a-a.a)*t)};}
void DrawTextC(Font f,const char* txt,float cx,float y,float sz,float sp,Color c){
    Vector2 s=MeasureTextEx(f,txt,sz,sp);DrawTextEx(f,txt,{cx-s.x*0.5f,y},sz,sp,c);}
std::string TruncText(const char* txt,Font f,float sz,float sp,float maxW){
    std::string s=txt;
    if(MeasureTextEx(f,s.c_str(),sz,sp).x<=maxW) return s;
    while(!s.empty()){s.pop_back();if(MeasureTextEx(f,(s+"...").c_str(),sz,sp).x<=maxW)return s+"...";}
    return "...";
}
std::vector<std::string> WrapText(const std::string& text,Font f,float sz,float sp,float maxW){
    std::vector<std::string> lines;
    std::istringstream ss(text);std::string word,line;
    while(ss>>word){
        std::string test=line.empty()?word:line+" "+word;
        if(MeasureTextEx(f,test.c_str(),sz,sp).x>maxW&&!line.empty()){lines.push_back(line);line=word;}
        else line=test;
    }
    if(!line.empty()) lines.push_back(line);
    return lines;
}

// ── Particles ─────────────────────────────────────────────────
std::vector<Particle> gParts;
static void Spawn(){
    float cx=SW*0.5f,cy=SH*0.5f,angle=GetRandomValue(0,628)*0.01f,spd=GetRandomValue(5,30)*0.1f;
    Color cols[]={C_GOLD,C_GOLD2,C_WHITE,C_ACCENT};
    Particle p;p.x=cx+cosf(angle)*GetRandomValue(0,200);p.y=cy+sinf(angle)*GetRandomValue(0,150);
    p.vx=cosf(angle)*spd;p.vy=sinf(angle)*spd;p.r=GetRandomValue(1,3)*0.5f;
    p.maxLife=GetRandomValue(60,180)*0.016f;p.life=p.maxLife;p.col=cols[GetRandomValue(0,3)];
    gParts.push_back(p);
}
void UpdateParticles(float dt){
    if((int)gParts.size()<120&&GetRandomValue(0,3)==0) Spawn();
    for(auto& p:gParts){p.x+=p.vx*dt;p.y+=p.vy*dt;p.life-=dt;}
    gParts.erase(std::remove_if(gParts.begin(),gParts.end(),[](const Particle& p){return p.life<=0;}),gParts.end());
}
void DrawParticles(){for(auto& p:gParts) DrawCircleV({p.x,p.y},p.r,CA(p.col,p.life/p.maxLife*0.55f));}

// ── Chrome ────────────────────────────────────────────────────
void DrawFilm(int x,int y,int h,float t,bool right){
    int sw=28,sh=20,sg=10;DrawRectangle(x,y,sw,h,CA(C_GOLDDIM,0.12f));
    int off=(int)(fmodf(t*30.f,(float)(sh+sg)));
    for(int sy=y-(sh+sg)+off;sy<y+h+sh;sy+=sh+sg){
        DrawRectangle(x+4,sy,sw-8,sh,CA(C_GOLD,0.18f));
        DrawRectangleLinesEx({(float)(x+4),(float)sy,(float)(sw-8),(float)sh},0.8f,CA(C_GOLDDIM,0.4f));
    }
    DrawRectangle(right?x+sw-2:x,y,2,h,CA(C_GOLD,0.25f));
}
void DrawScanlines(){for(int y=0;y<SH;y+=3) DrawRectangle(0,y,SW,1,CA(C_BG,0.18f));}
void DrawGlowRect(Rectangle r,Color col,float str,int layers){
    for(int i=layers;i>=1;i--){float e=(float)i*3.f;DrawRectangleLinesEx({r.x-e,r.y-e,r.width+e*2,r.height+e*2},1.f,CA(col,str*(1.f-(float)i/(float)(layers+1))*0.5f));}
}
void DrawHeader(const char* sub,float t,bool showLogout){
    DrawRectangle(28,0,SW-56,80,CA(C_PANEL,0.97f));DrawRectangle(28,78,SW-56,2,CA(C_GOLDDIM,0.5f));
    DrawTextC(fTitle,"MOVIX",SW*0.5f,12,38,4,C_GOLD2);
    DrawTextEx(fUI,sub,{48,30},11,3,CA(C_GOLD,0.60f));
}

// ── Widgets ───────────────────────────────────────────────────
void FieldInit(Field& f,Rectangle r,const char* lbl,bool pwd){f.rect=r;f.buf="";f.active=false;f.isPwd=pwd;f.blink=0;f.hoverT=0;f.label=lbl;}
void FieldUpdate(Field& f,float dt){
    f.blink+=dt;
    if(f.active){int k=GetCharPressed();while(k>0){if(k>=32&&k<=126&&(int)f.buf.size()<48)f.buf+=(char)k;k=GetCharPressed();}if(IsKeyPressed(KEY_BACKSPACE)&&!f.buf.empty())f.buf.pop_back();}
    bool h=CheckCollisionPointRec(GetMousePosition(),f.rect);f.hoverT+=dt*(h?6.f:-6.f);f.hoverT=std::clamp(f.hoverT,0.f,1.f);
}
void FieldDraw(const Field& f){
    float gl=f.active?1.f:f.hoverT*0.4f;
    Color b=f.active?C_GOLD:LerpC(CA(C_GREY,0.5f),CA(C_GOLDDIM,0.8f),f.hoverT);
    if(gl>0.01f) DrawGlowRect(f.rect,C_GOLD,gl*0.6f,5);
    DrawRectangleRec(f.rect,C_INPUTBG);DrawRectangleLinesEx(f.rect,f.active?1.8f:1.2f,b);
    bool up=f.active||!f.buf.empty();
    float lsz=up?11.f:15.f,ly=up?f.rect.y-18.f:f.rect.y+(f.rect.height-16.f)*0.5f;
    DrawTextEx(fUIBold,f.label,{f.rect.x+2,ly},lsz,2,up?CA(C_GOLD,0.9f):CA(C_GREY,0.8f));
    std::string shown=f.isPwd?std::string(f.buf.size(),'*'):f.buf;
    float tsz=18.f,ty=f.rect.y+(f.rect.height-tsz)*0.5f;
    DrawTextEx(fUI,shown.c_str(),{f.rect.x+14,ty},tsz,1,C_WHITE);
    if(f.active&&fmodf(f.blink,1.f)<0.55f){Vector2 ms=MeasureTextEx(fUI,shown.c_str(),tsz,1);DrawRectangle((int)(f.rect.x+14+ms.x+2),(int)ty,2,(int)tsz,C_GOLD);}
}
void BtnInit(Button& b,Rectangle r,const char* t){b.rect=r;b.hoverT=0;b.pressT=0;b.text=t;}
bool BtnUpdate(Button& b,float dt){
    bool h=CheckCollisionPointRec(GetMousePosition(),b.rect);
    bool cl=h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    b.hoverT+=dt*(h?8.f:-8.f);b.hoverT=std::clamp(b.hoverT,0.f,1.f);
    if(cl) b.pressT=0.12f;if(b.pressT>0) b.pressT-=dt;
    return cl;
}
void BtnDraw(const Button& b){
    float pulse=sinf(GetTime()*2.5f)*0.5f+0.5f;Color base=LerpC(C_GOLD,C_GOLD2,b.hoverT);
    float sc=b.pressT>0?0.97f:1.f;
    Rectangle r={b.rect.x+b.rect.width*(1-sc)*0.5f,b.rect.y+b.rect.height*(1-sc)*0.5f,b.rect.width*sc,b.rect.height*sc};
    DrawGlowRect(r,C_GOLD,0.3f+b.hoverT*0.5f+pulse*0.15f,7);DrawRectangleRec(r,base);
    DrawRectangleGradientV((int)r.x,(int)r.y,(int)r.width,(int)r.height/2,CA(C_WHITE,0.14f),CA(C_WHITE,0.f));
    float sz=17.f;Vector2 ts=MeasureTextEx(fUIBold,b.text,sz,3);
    DrawTextEx(fUIBold,b.text,{r.x+(r.width-ts.x)*0.5f,r.y+(r.height-ts.y)*0.5f},sz,3,C_BG);
}
void BtnDrawDisabled(const Button& b){
    DrawRectangleRounded(b.rect,0.3f,8,CA(C_PANEL,0.4f));DrawRectangleRoundedLines(b.rect,0.3f,8,1.f,CA(C_GREY,0.25f));
    float lw=MeasureTextEx(fUIBold,b.text,13.f,2).x;
    DrawTextEx(fUIBold,b.text,{b.rect.x+(b.rect.width-lw)*0.5f,b.rect.y+(b.rect.height-13.f)*0.5f},13.f,2,CA(C_GREY,0.30f));
}

// ── Stars ─────────────────────────────────────────────────────
void DrawStars(float x,float y,float rating,float sz,Color col){
    float stars=rating/2.f;
    for(int i=0;i<5;i++){
        float filled=std::clamp(stars-(float)i,0.f,1.f);
        float cx2=x+(float)i*(sz+3.f)+sz*0.5f,cy2=y+sz*0.5f,outer=sz*0.5f,inner=outer*0.4f;
        for(int p=0;p<5;p++){
            float a0=-1.5707963f+(float)p*1.2566370f,a1=a0+0.6283185f,am=a0+0.3141592f;
            DrawTriangle({cx2+cosf(a0)*outer,cy2+sinf(a0)*outer},{cx2,cy2},{cx2+cosf(am)*inner,cy2+sinf(am)*inner},CA(col,filled*0.9f));
            DrawTriangle({cx2+cosf(am)*inner,cy2+sinf(am)*inner},{cx2,cy2},{cx2+cosf(a1)*outer,cy2+sinf(a1)*outer},CA(col,filled*0.9f));
        }
        if(filled<1.f) DrawCircleLines((int)cx2,(int)cy2,outer*0.85f,CA(col,0.25f));
    }
}
int DrawInteractiveStars(float x,float y,int current,float sz){
    Vector2 m=GetMousePosition(); int clicked=current;
    for(int i=0;i<5;i++){
        float cx2=x+(float)i*(sz+4.f)+sz*0.5f,cy2=y+sz*0.5f,outer=sz*0.5f;
        bool hover=CheckCollisionPointCircle(m,{cx2,cy2},outer+2.f);
        Color col=i<current?C_GOLD2:(hover?CA(C_GOLD,0.7f):CA(C_GREY,0.4f));
        for(int p=0;p<5;p++){
            float a0=-1.5707963f+(float)p*1.2566370f,a1=a0+0.6283185f,am=a0+0.3141592f,inner=outer*0.4f;
            DrawTriangle({cx2+cosf(a0)*outer,cy2+sinf(a0)*outer},{cx2,cy2},{cx2+cosf(am)*inner,cy2+sinf(am)*inner},col);
            DrawTriangle({cx2+cosf(am)*inner,cy2+sinf(am)*inner},{cx2,cy2},{cx2+cosf(a1)*outer,cy2+sinf(a1)*outer},col);
        }
        if(hover&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) clicked=i+1;
    }
    return clicked;
}

// ── Logo / Clapper ────────────────────────────────────────────
void DrawLogo(float cx,float y,float t){
    float pulse=0.55f+0.2f*sinf(t*1.8f);Vector2 ts=MeasureTextEx(fTitle,"MOVIX",58,5);
    for(int i=8;i>=1;i--){float e=(float)i*5.f;DrawRectangle((int)(cx-ts.x*0.5f-e),(int)(y-e),(int)(ts.x+e*2),(int)(58+e*2),CA(C_GOLD,pulse*0.035f));}
    DrawTextC(fTitle,"MOVIX",cx+2,y+2,58,5,CA(C_BG,0.8f));DrawTextC(fTitle,"MOVIX",cx,y,58,5,C_GOLD2);
    DrawTextC(fUI,"CINEMA MANAGEMENT SYSTEM",cx,y+70,13,5,CA(C_GREYLT,0.7f));
    DrawLineEx({cx-100,y+90},{cx+100,y+90},1.f,CA(C_GOLDDIM,0.6f));DrawCircle((int)cx,(int)(y+90),3,CA(C_GOLD,0.7f));
}
void DrawClapper(float cx,float y,float t){
    float w=56,h=42,x=cx-w*0.5f;
    DrawRectangle((int)x,(int)(y+14),(int)w,(int)h,CA(C_GOLDDIM,0.2f));
    DrawRectangleLinesEx({x,(float)(y+14),w,h},1.5f,CA(C_GOLD,0.5f));
    for(int i=0;i<5;i++){float sx=x+(float)i*12.f;DrawLine((int)sx,(int)(y+14),(int)(sx+8),(int)y,CA(C_GOLD,0.55f));}
    DrawLine((int)x,(int)(y+14),(int)(x+w),(int)(y+14),CA(C_GOLD,0.55f));
    DrawCircleLinesV({cx,y+14+h*0.5f},10,CA(C_ACCENT,0.35f));DrawCircle((int)cx,(int)(y+14+h*0.5f),5,CA(C_ACCENT,0.2f));(void)t;
}
