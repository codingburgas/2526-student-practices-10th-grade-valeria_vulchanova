#include "theme.h"
#include "screens.h"
#include "../bll/auth_service.h"
#include "../bll/session.h"
#include <cmath>
#include <algorithm>

enum LTab{TAB_L,TAB_R};

static LTab DrawTabBar(float px,float pw,float ty,LTab cur,float dt,float& lt,float& rt){
    float tw=(pw-20.f)*0.5f,th=36.f,lx=px+10.f,rx=lx+tw+2.f;
    bool hl=CheckCollisionPointRec(GetMousePosition(),{lx,ty,tw,th});
    bool hr=CheckCollisionPointRec(GetMousePosition(),{rx,ty,tw,th});
    lt+=dt*(hl?8.f:-8.f);rt+=dt*(hr?8.f:-8.f);
    lt=std::clamp(lt,0.f,1.f);rt=std::clamp(rt,0.f,1.f);
    LTab next=cur;
    if(hl&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) next=TAB_L;
    if(hr&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) next=TAB_R;
    for(int i=0;i<2;i++){
        float x=(i==0)?lx:rx;bool act=(cur==(LTab)i);float hv=(i==0)?lt:rt;
        const char* lbl=(i==0)?"LOGIN":"REGISTER";
        DrawRectangleRec({x,ty,tw,th},act?CA(C_GOLD,0.18f):CA(C_PANEL,0.70f+hv*0.20f));
        DrawRectangleLinesEx({x,ty,tw,th},act?1.6f:1.f,act?CA(C_GOLD,0.70f):CA(C_GOLDDIM,0.30f+hv*0.30f));
        float lw=MeasureTextEx(fUIBold,lbl,11.f,2).x;
        DrawTextEx(fUIBold,lbl,{x+(tw-lw)*0.5f,ty+(th-11.f)*0.5f},11.f,2,act?C_GOLD2:CA(C_GREYLT,0.60f+hv*0.30f));
        if(act) DrawRectangle((int)x,(int)(ty+th-2),(int)tw,2,CA(C_GOLD,0.80f));
    }
    return next;
}

void RunLoginScreen(){
    float pw=480.f,ph=600.f,px=(SW-pw)*0.5f,py=(SH-ph)*0.5f,cx=SW*0.5f;
    LTab tab=TAB_L;float lt=1.f,rt=0.f;

    Field uF,pF,ruF,rpF,rcF;
    FieldInit(uF, {px+40,py+298,pw-80,50},"USERNAME",false);
    FieldInit(pF, {px+40,py+382,pw-80,50},"PASSWORD",true);
    FieldInit(ruF,{px+40,py+282,pw-80,50},"USERNAME",false);
    FieldInit(rpF,{px+40,py+358,pw-80,50},"PASSWORD",true);
    FieldInit(rcF,{px+40,py+434,pw-80,50},"CONFIRM PASSWORD",true);
    Button lBtn,rBtn;
    BtnInit(lBtn,{px+40,py+460,pw-80,52},"SIGN IN");
    BtnInit(rBtn,{px+40,py+516,pw-80,52},"CREATE ACCOUNT");

    std::string msg; Color mc=C_RED; float mAlpha=0.f,mTimer=0.f;
    bool loggedIn=false; float lanim=0.f;

    while(!WindowShouldClose()){
        float dt=GetFrameTime(),t=GetTime();
        UpdateParticles(dt);

        // Field updates
        if(tab==TAB_L){
            FieldUpdate(uF,dt);FieldUpdate(pF,dt);
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                uF.active=CheckCollisionPointRec(GetMousePosition(),uF.rect);
                pF.active=CheckCollisionPointRec(GetMousePosition(),pF.rect);
            }
            if(IsKeyPressed(KEY_TAB)){if(uF.active){uF.active=false;pF.active=true;}else{pF.active=false;uF.active=true;}}
        } else {
            FieldUpdate(ruF,dt);FieldUpdate(rpF,dt);FieldUpdate(rcF,dt);
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                ruF.active=CheckCollisionPointRec(GetMousePosition(),ruF.rect);
                rpF.active=CheckCollisionPointRec(GetMousePosition(),rpF.rect);
                rcF.active=CheckCollisionPointRec(GetMousePosition(),rcF.rect);
            }
            if(IsKeyPressed(KEY_TAB)){
                if(ruF.active){ruF.active=false;rpF.active=true;}
                else if(rpF.active){rpF.active=false;rcF.active=true;}
                else{rcF.active=false;ruF.active=true;}
            }
        }

        // Logic
        if(tab==TAB_L&&!loggedIn){
            bool cl=BtnUpdate(lBtn,dt)||((uF.active||pF.active)&&IsKeyPressed(KEY_ENTER));
            if(cl&&!uF.buf.empty()){
                if(AuthLogin(uF.buf,pF.buf)) loggedIn=true;
                else{msg="Invalid username or password.";mc=C_RED;mAlpha=1.f;mTimer=3.5f;}
            }
        }
        if(tab==TAB_R){
            bool cl=BtnUpdate(rBtn,dt)||(rcF.active&&IsKeyPressed(KEY_ENTER));
            if(cl){
                if(ruF.buf.empty()||rpF.buf.empty())      {msg="Fields cannot be empty.";mc=C_RED;mAlpha=1.f;mTimer=3.f;}
                else if(rpF.buf!=rcF.buf)                  {msg="Passwords do not match.";mc=C_RED;mAlpha=1.f;mTimer=3.f;}
                else if(rpF.buf.size()<4)                  {msg="Password: min 4 characters.";mc=C_RED;mAlpha=1.f;mTimer=3.f;}
                else if(!AuthRegister(ruF.buf,rpF.buf))    {msg="Username already taken.";mc=C_RED;mAlpha=1.f;mTimer=3.f;}
                else{msg="Account created! Log in now.";mc=C_GREEN;mAlpha=1.f;mTimer=3.5f;
                     tab=TAB_L;uF.buf=ruF.buf;pF.buf="";ruF.buf=rpF.buf=rcF.buf="";}
            }
        }
        if(mTimer>0){mTimer-=dt;mAlpha=std::min(1.f,mTimer/0.4f);}
        if(loggedIn){lanim=std::min(1.f,lanim+dt*1.8f);if(lanim>=1.f){if(gIsAdmin)RunAdminScreen();RunMovieScreen();return;}}

        BeginDrawing();ClearBackground(C_BG);DrawParticles();
        DrawFilm(0,0,SH,t,false);DrawFilm(SW-28,0,SH,t,true);
        DrawRectangle(38,24,SW-76,1,CA(C_GOLDDIM,0.45f));
        DrawRectangle(38,SH-26,SW-76,1,CA(C_GOLDDIM,0.45f));
        DrawTextEx(fUI,"MOVIX",{50,12},11,3,CA(C_GOLD,0.40f));
        DrawTextEx(fUI,"v4.0",{(float)(SW-80),12},11,2,CA(C_GREY,0.38f));

        for(int i=6;i>=1;i--){float e=(float)i*8.f;DrawRectangle((int)(px-e),(int)(py-e),(int)(pw+e*2),(int)(ph+e*2),CA(C_GOLD,0.012f));}
        DrawRectangleRec({px,py,pw,ph},C_PANEL);
        float cs=18.f;
        auto corner=[&](float x,float y,float dx,float dy){DrawLineEx({x,y},{x+dx*cs,y},1.5f,CA(C_GOLD,0.7f));DrawLineEx({x,y},{x,y+dy*cs},1.5f,CA(C_GOLD,0.7f));};
        corner(px,py,1,1);corner(px+pw,py,-1,1);corner(px,py+ph,1,-1);corner(px+pw,py+ph,-1,-1);
        DrawRectangleLinesEx({px,py,pw,ph},0.8f,CA(C_GOLDDIM,0.30f));
        float pulse=sinf(t*2.f)*0.3f+0.7f;
        DrawRectangleGradientH((int)px,(int)py,(int)(pw/2),3,CA(C_BG,0),CA(C_GOLD,pulse));
        DrawRectangleGradientH((int)(px+pw/2),(int)py,(int)(pw/2),3,CA(C_GOLD,pulse),CA(C_BG,0));
        DrawClapper(cx,py+20,t);DrawLogo(cx,py+80,t);

        LTab nt=DrawTabBar(px,pw,py+196.f,tab,dt,lt,rt);
        if(nt!=tab){tab=nt;msg="";mAlpha=0.f;uF.active=pF.active=ruF.active=rpF.active=rcF.active=false;}

        if(tab==TAB_L){FieldDraw(uF);FieldDraw(pF);BtnDraw(lBtn);}
        else{FieldDraw(ruF);FieldDraw(rpF);FieldDraw(rcF);BtnDraw(rBtn);}

        float msgY=py+ph-28.f;
        if(mAlpha>0.01f){float mw=MeasureTextEx(fUI,msg.c_str(),11.f,1).x;DrawTextEx(fUI,msg.c_str(),{cx-mw*0.5f,msgY},11.f,1,CA(mc,mAlpha));}
        else DrawTextC(fUI,tab==TAB_L?"TAB-switch  ENTER-login":"TAB-next  ENTER-register",cx,msgY,10.f,1,CA(C_GREY,0.40f));

        if(loggedIn){
            float a=lanim;DrawRectangle(0,0,SW,SH,CA(C_BG,a*0.92f));
            if(a>0.35f){float fa=(a-0.35f)/0.65f;DrawGlowRect({cx-180,SH*0.5f-65,360,130},C_GOLD,fa*0.8f,8);
                DrawRectangle((int)(cx-180),(int)(SH*0.5f-65),360,130,CA(C_PANEL,fa));
                std::string wlc=gIsAdmin?"Welcome, Admin":("Welcome, "+gCurrentUser);
                DrawTextC(fTitle,wlc.c_str(),cx,SH*0.5f-52,26,4,CA(C_GOLD2,fa));
                DrawTextC(fUI,gIsAdmin?"Administrator":"Movix Cinema Suite",cx,SH*0.5f+4,15,2,CA(C_WHITE,fa*0.8f));
            }
        }
        DrawScanlines();
        DrawTextC(fUI,"(c) 2025 Movix - Cinema Management Suite",cx,SH-18,10,1,CA(C_GREY,0.32f));
        EndDrawing();
    }
}
