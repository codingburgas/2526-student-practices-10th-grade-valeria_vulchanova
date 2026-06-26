#include "theme.h"
#include "screens.h"
#include "win_dialog.h"      // isolated – no windows.h here, avoids ShowCursor conflict
#include "../bll/session.h"
#include "../bll/stats_service.h"
#include "../bll/movie_service.h"
#include "../dal/user_repo.h"
#include "../dal/movie_repo.h"
#include "../dal/reservation_repo.h"
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <vector>

static Color Ac5(const MovieEntry& e){return {(unsigned char)e.ac_r,(unsigned char)e.ac_g,(unsigned char)e.ac_b,255};}

enum ATab{AT_USERS,AT_RESERVATIONS,AT_MOVIES,AT_STATS,AT_COUNT};
static const char* ATAB_NAMES[]={"USERS","RESERVATIONS","MOVIES","STATISTICS"};

static ATab DrawATabs(ATab cur,float* hvs,float dt){
    float tw=(float)(SW-56)/AT_COUNT-4.f,ty=82.f,th=34.f;ATab next=cur;
    for(int i=0;i<AT_COUNT;i++){
        float tx=28.f+i*(tw+4.f);
        bool h=CheckCollisionPointRec(GetMousePosition(),{tx,ty,tw,th});
        hvs[i]+=dt*(h?8.f:-8.f);hvs[i]=std::clamp(hvs[i],0.f,1.f);
        if(h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) next=(ATab)i;
        bool act=(cur==(ATab)i);
        DrawRectangleRec({tx,ty,tw,th},act?CA(C_GOLD,0.18f):CA(C_PANEL,0.70f+hvs[i]*0.20f));
        DrawRectangleLinesEx({tx,ty,tw,th},act?1.6f:1.f,act?CA(C_GOLD,0.70f):CA(C_GOLDDIM,0.28f+hvs[i]*0.28f));
        float lw=MeasureTextEx(fUIBold,ATAB_NAMES[i],10.5f,2).x;
        DrawTextEx(fUIBold,ATAB_NAMES[i],{tx+(tw-lw)*0.5f,ty+(th-10.5f)*0.5f},10.5f,2,act?C_GOLD2:CA(C_GREYLT,0.55f+hvs[i]*0.35f));
        if(act) DrawRectangle((int)tx,(int)(ty+th-2),(int)tw,2,CA(C_GOLD,0.85f));
    }
    return next;
}

static bool ConfirmDlg(const char* q,float& pt,float dt){
    if(pt<=0.f) return false;
    float a=std::min(1.f,pt/0.25f)*std::min(1.f,pt);
    DrawRectangle(0,0,SW,SH,CA(C_BG,a*0.80f));
    DrawRectangleRounded({SW*0.5f-210.f,SH*0.5f-80.f,420.f,160.f},0.08f,8,CA(C_PANEL,0.98f));
    DrawRectangleLinesEx({SW*0.5f-210.f,SH*0.5f-80.f,420.f,160.f},2.f,CA(C_RED,a));
    DrawTextC(fUIBold,q,SW*0.5f,SH*0.5f-60.f,12.f,2,CA(C_WHITE,a));
    Rectangle yes={SW*0.5f+10.f,SH*0.5f+10.f,150.f,36.f};
    Rectangle no ={SW*0.5f-160.f,SH*0.5f+10.f,150.f,36.f};
    bool hy=CheckCollisionPointRec(GetMousePosition(),yes);
    bool hn=CheckCollisionPointRec(GetMousePosition(),no);
    DrawRectangleRec(yes,CA(C_RED,hy?0.70f:0.40f));DrawRectangleLinesEx(yes,1.f,CA(C_RED,0.80f));
    DrawTextC(fUIBold,"YES",yes.x+yes.width*0.5f,yes.y+10.f,11.f,2,CA(C_WHITE,a));
    DrawRectangleRec(no,CA(C_PANEL,0.85f));DrawRectangleLinesEx(no,1.f,CA(C_GREY,0.50f));
    DrawTextC(fUIBold,"CANCEL",no.x+no.width*0.5f,no.y+10.f,11.f,2,CA(C_GREYLT,a));
    if(hy&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){pt=0.f;return true;}
    if((hn&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON))||IsKeyPressed(KEY_ESCAPE)) pt=0.f;
    return false;
}

void RunAdminScreen(){
    float fadeIn=0.f;ATab curTab=AT_USERS;float tabHvs[AT_COUNT]={};
    float scrollY[AT_COUNT]={},scrollT[AT_COUNT]={};
    float confirmT=0.f;int confirmIdx=-1;
    std::string flash;float flashT=0.f;Color flashC=C_GREEN;
    Button backBtn;BtnInit(backBtn,{36.f,22.f,150.f,36.f},"< BACK TO CINEMA");

    // Add movie form
    bool addFormOpen=false;int selTheme=0;
    Field fTitle2,fGenre,fDir,fCast,fDur,fYear,fRating,fPrice,fDesc;
    auto initFrm=[&](){
        float fw=420.f,fx=SW*0.5f-fw*0.5f,fy=105.f,fh=40.f,fg=12.f;
        FieldInit(fTitle2,{fx,fy,fw,fh},"TITLE",false);fy+=fh+fg;
        FieldInit(fGenre, {fx,fy,fw*0.5f-4,fh},"GENRE",false);
        FieldInit(fDir,   {fx+fw*0.5f+4,fy,fw*0.5f-4,fh},"DIRECTOR",false);fy+=fh+fg;
        FieldInit(fCast,  {fx,fy,fw,fh},"CAST",false);fy+=fh+fg;
        FieldInit(fDur,   {fx,fy,fw*0.5f-4,fh},"DURATION",false);
        FieldInit(fYear,  {fx+fw*0.5f+4,fy,fw*0.5f-4,fh},"YEAR",false);fy+=fh+fg;
        FieldInit(fRating,{fx,fy,fw*0.5f-4,fh},"RATING (0-10)",false);
        FieldInit(fPrice, {fx+fw*0.5f+4,fy,fw*0.5f-4,fh},"PRICE (BGN)",false);fy+=fh+fg;
        FieldInit(fDesc,  {fx,fy,fw,50.f},"SYNOPSIS",false);
    };
    initFrm();

    while(!WindowShouldClose()){
        float dt=GetFrameTime(),t=GetTime();
        fadeIn=std::min(1.f,fadeIn+dt*2.f);
        UpdateParticles(dt);
        if(flashT>0.f) flashT-=dt;

        if(!addFormOpen){
            if(BtnUpdate(backBtn,dt)||IsKeyPressed(KEY_ESCAPE)) return;
            for(int i=0;i<AT_COUNT;i++){scrollT[i]-=GetMouseWheelMove()*50.f;scrollT[i]=std::clamp(scrollT[i],0.f,3000.f);scrollY[i]+=(scrollT[i]-scrollY[i])*std::min(1.f,dt*12.f);}
        }

        BeginDrawing();ClearBackground(C_BG);DrawParticles();
        DrawFilm(0,0,SH,t,false);DrawFilm(SW-28,0,SH,t,true);
        DrawRectangle(28,0,SW-56,80,CA(C_PANEL,0.97f));DrawRectangle(28,78,SW-56,2,CA(C_GOLDDIM,0.5f));
        DrawTextC(fTitle,"MOVIX",SW*0.5f,12,36,4,C_GOLD2);
        DrawTextEx(fUI,"ADMIN PANEL",{50.f,30.f},11.f,3,CA(C_GOLD,0.60f));
        char ub[64];snprintf(ub,sizeof(ub),"Logged in as: %s",gCurrentUser.c_str());
        float ubw=MeasureTextEx(fUI,ub,9.5f,1).x;DrawTextEx(fUI,ub,{(float)(SW-ubw-40),46.f},9.5f,1,CA(C_GREYLT,0.45f));
        BtnDraw(backBtn);

        ATab nt=DrawATabs(curTab,tabHvs,dt);
        if(nt!=curTab){curTab=nt;confirmT=0.f;confirmIdx=-1;}

        float CY=120.f,CH2=SH-CY-14.f;

        // ══════ USERS TAB ══════════════════════════════════════
        if(curTab==AT_USERS){
            auto users=UserRepo_LoadAll();
            float rH=44.f,y0=CY+8.f-scrollY[0];
            BeginScissorMode(28,(int)CY,SW-56,(int)CH2);
            // Admin row (built-in)
            if(y0>CY-rH&&y0<CY+CH2){
                DrawRectangle(28,(int)y0,SW-56,(int)(rH-2),CA(C_GOLD,0.08f));
                DrawRectangleLinesEx({28.f,y0,(float)(SW-56),rH-2.f},0.8f,CA(C_GOLDDIM,0.30f));
                DrawCircle(50,(int)(y0+rH*0.5f-1),7,CA(C_GOLD,0.80f));
                DrawTextEx(fUIBold,"admin",{64.f,y0+14.f},12.f,1,C_GOLD2);
                DrawTextEx(fUI,"Built-in administrator",{200.f,y0+15.f},9.5f,1,CA(C_GREY,0.50f));
                DrawRectangle(SW-110,(int)(y0+12),80,20,CA(C_GOLD,0.20f));
                DrawTextEx(fUIBold,"ADMIN",{(float)(SW-105.f),(float)(y0+15)},9.f,2,CA(C_GOLD,0.80f));
            }
            y0+=rH;
            for(int i=0;i<(int)users.size();i++){
                float ry=y0+i*rH;if(ry+rH<CY||ry>CY+CH2) continue;
                bool h=CheckCollisionPointRec(GetMousePosition(),{28.f,ry,(float)(SW-56),rH-2.f});
                DrawRectangle(28,(int)ry,SW-56,(int)(rH-2),CA(C_PANEL,h?0.88f:0.55f+(i%2)*0.08f));
                DrawRectangleLinesEx({28.f,ry,(float)(SW-56),rH-2.f},0.7f,CA(C_GOLDDIM,0.18f));
                DrawCircle(50,(int)(ry+rH*0.5f-1),7,CA(C_ACCENT,0.55f));
                DrawTextEx(fUIBold,users[i].username.c_str(),{64.f,ry+14.f},12.f,1,C_WHITE);
                int uRes=(int)ResRepo_ForUser(users[i].username).size();
                char rb[32];snprintf(rb,sizeof(rb),"%d reservation%s",uRes,uRes==1?"":"s");
                DrawTextEx(fUI,rb,{240.f,ry+15.f},10.f,1,CA(C_GREYLT,0.55f));
                Rectangle db={SW-118.f,ry+9.f,88.f,26.f};
                bool dh=CheckCollisionPointRec(GetMousePosition(),db);
                DrawRectangleRec(db,CA(C_RED,dh?0.55f:0.25f));DrawRectangleLinesEx(db,0.8f,CA(C_RED,0.70f));
                float dlw=MeasureTextEx(fUIBold,"DELETE",9.5f,1).x;DrawTextEx(fUIBold,"DELETE",{db.x+(88.f-dlw)*0.5f,db.y+8.f},9.5f,1,CA(C_WHITE,0.90f));
                if(dh&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)&&confirmT<=0.f){confirmIdx=i;confirmT=999.f;}
            }
            EndScissorMode();
            if(confirmT>0.f&&confirmIdx>=0&&confirmIdx<(int)users.size()){
                char q[128];snprintf(q,sizeof(q),"Delete user \"%s\"?",users[confirmIdx].username.c_str());
                if(ConfirmDlg(q,confirmT,dt)){UserRepo_Delete(users[confirmIdx].username);flash="User deleted.";flashT=2.5f;flashC=C_ACCENT;confirmIdx=-1;}
            }
        }

        // ══════ RESERVATIONS TAB ═══════════════════════════════
        else if(curTab==AT_RESERVATIONS){
            auto allR=ResRepo_LoadAll();float rH=46.f,y0=CY+8.f-scrollY[1];
            char th[48];snprintf(th,sizeof(th),"Total: %d reservation%s",(int)allR.size(),(int)allR.size()==1?"":"s");
            DrawTextEx(fUIBold,th,{36.f,y0+4.f},11.f,2,CA(C_GOLD,0.70f));y0+=38.f;
            BeginScissorMode(28,(int)CY,SW-56,(int)CH2);
            for(int m=0;m<MovieRepo_Count();m++){
                float ry=y0+m*rH;if(ry+rH<CY||ry>CY+CH2) continue;
                const MovieEntry& e=MovieRepo_Get(m);Color ac=Ac5(e);
                int cnt=ResRepo_CountMovie(m);
                bool h=CheckCollisionPointRec(GetMousePosition(),{28.f,ry,(float)(SW-56),rH-2.f});
                DrawRectangle(28,(int)ry,SW-56,(int)(rH-2),CA(C_PANEL,h?0.88f:0.55f+(m%2)*0.08f));
                DrawRectangleLinesEx({28.f,ry,(float)(SW-56),rH-2.f},0.7f,CA(ac,0.22f));
                DrawRectangle(28,(int)ry,4,(int)(rH-2),CA(ac,0.55f));
                DrawTextEx(fUIBold,TruncText(e.title,fUIBold,11.f,1,450.f).c_str(),{40.f,ry+8.f},11.f,1,C_WHITE);
                char cb2[32];snprintf(cb2,sizeof(cb2),"%d seat%s booked",cnt,cnt==1?"":"s");
                DrawTextEx(fUI,cb2,{40.f,ry+26.f},9.5f,1,cnt>0?CA(C_GOLD,0.75f):CA(C_GREY,0.45f));
                Rectangle cb3={SW-118.f,ry+9.f,88.f,26.f};
                bool ch=cnt>0&&CheckCollisionPointRec(GetMousePosition(),cb3);
                DrawRectangleRec(cb3,CA(C_RED,cnt>0?(ch?0.55f:0.25f):0.10f));
                DrawRectangleLinesEx(cb3,0.8f,CA(C_RED,cnt>0?0.65f:0.20f));
                float clw=MeasureTextEx(fUIBold,"CLEAR",9.5f,1).x;DrawTextEx(fUIBold,"CLEAR",{cb3.x+(88.f-clw)*0.5f,cb3.y+8.f},9.5f,1,CA(C_WHITE,cnt>0?0.90f:0.30f));
                if(ch&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)&&confirmT<=0.f){confirmIdx=m;confirmT=999.f;}
            }
            EndScissorMode();
            if(confirmT>0.f&&confirmIdx>=0){
                const MovieEntry& e=MovieRepo_Get(confirmIdx);char q[128];snprintf(q,sizeof(q),"Clear reservations for \"%s\"?",e.title);
                if(ConfirmDlg(q,confirmT,dt)){ResRepo_ClearMovie(confirmIdx);flash="Reservations cleared.";flashT=2.5f;flashC=C_ACCENT;confirmIdx=-1;}
            }
            // Clear all
            Rectangle ca={SW-178.f,(float)(SH-58),150.f,40.f};
            bool cah=CheckCollisionPointRec(GetMousePosition(),ca);
            DrawRectangleRec(ca,CA(C_RED,cah?0.60f:0.30f));DrawRectangleLinesEx(ca,1.f,CA(C_RED,0.75f));
            float calw=MeasureTextEx(fUIBold,"CLEAR ALL",11.f,2).x;DrawTextEx(fUIBold,"CLEAR ALL",{ca.x+(150.f-calw)*0.5f,ca.y+12.f},11.f,2,CA(C_WHITE,0.90f));
            if(cah&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)&&confirmT<=0.f){confirmIdx=-999;confirmT=999.f;}
            if(confirmT>0.f&&confirmIdx==-999){if(ConfirmDlg("Clear ALL reservations?",confirmT,dt)){ResRepo_ClearAll();flash="All reservations cleared.";flashT=2.5f;flashC=C_ACCENT;confirmIdx=-1;}}
        }

        // ══════ MOVIES TAB ════════════════════════════════════
        else if(curTab==AT_MOVIES){
            if(!addFormOpen){
                float rH=50.f,y0=CY+8.f-scrollY[2];
                BeginScissorMode(28,(int)CY,SW-56,(int)CH2);
                for(int m=0;m<MovieRepo_Count();m++){
                    float ry=y0+m*rH;if(ry+rH<CY||ry>CY+CH2) continue;
                    const MovieEntry& e=MovieRepo_Get(m);Color ac=Ac5(e);
                    bool h=CheckCollisionPointRec(GetMousePosition(),{28.f,ry,(float)(SW-56),rH-2.f});
                    DrawRectangle(28,(int)ry,SW-56,(int)(rH-2),CA(C_PANEL,h?0.88f:0.55f+(m%2)*0.08f));
                    DrawRectangleLinesEx({28.f,ry,(float)(SW-56),rH-2.f},0.7f,CA(ac,0.22f));
                    DrawRectangle(28,(int)ry,4,(int)(rH-2),CA(ac,0.60f));
                    bool hasPoster=MovieSvc_HasPoster(e.id);
                    if(hasPoster){DrawCircle(46,(int)(ry+rH*0.5f-1),5,CA(C_GREEN,0.80f));}
                    DrawTextEx(fUIBold,TruncText(e.title,fUIBold,11.f,1,380.f).c_str(),{56.f,ry+8.f},11.f,1,C_WHITE);
                    char sub2[80];snprintf(sub2,sizeof(sub2),"%s  |  %s  |  %.1f  |  %.2f BGN",e.genre,e.year,e.rating,e.price);
                    DrawTextEx(fUI,sub2,{56.f,ry+28.f},8.5f,1,CA(C_GREYLT,0.50f));
                    // Image button
                    Rectangle ib={SW-230.f,ry+11.f,100.f,26.f};
                    bool ih=CheckCollisionPointRec(GetMousePosition(),ib);
                    DrawRectangleRec(ib,CA(C_ACCENT,ih?0.55f:0.25f));DrawRectangleLinesEx(ib,0.8f,CA(C_ACCENT,0.70f));
                    float ilw=MeasureTextEx(fUIBold,"SET IMAGE",8.5f,1).x;DrawTextEx(fUIBold,"SET IMAGE",{ib.x+(100.f-ilw)*0.5f,ib.y+8.f},8.5f,1,CA(C_WHITE,0.90f));
                    if(ih&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                        std::string src=PickImageFile();
                        if(!src.empty()){MovieSvc_SetPoster(e.id,src);flash="Poster updated!";flashT=2.5f;flashC=C_GREEN;}
                    }
                    // Delete button
                    Rectangle db2={SW-118.f,ry+11.f,88.f,26.f};
                    bool dh=CheckCollisionPointRec(GetMousePosition(),db2);
                    DrawRectangleRec(db2,CA(C_RED,dh?0.55f:0.25f));DrawRectangleLinesEx(db2,0.8f,CA(C_RED,0.70f));
                    float dlw2=MeasureTextEx(fUIBold,"DELETE",9.5f,1).x;DrawTextEx(fUIBold,"DELETE",{db2.x+(88.f-dlw2)*0.5f,db2.y+8.f},9.5f,1,CA(C_WHITE,0.90f));
                    if(dh&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)&&confirmT<=0.f){confirmIdx=m;confirmT=999.f;}
                }
                EndScissorMode();
                if(confirmT>0.f&&confirmIdx>=0&&confirmIdx<MovieRepo_Count()){
                    const MovieEntry& e=MovieRepo_Get(confirmIdx);char q2[128];snprintf(q2,sizeof(q2),"Delete \"%s\"?",e.title);
                    if(ConfirmDlg(q2,confirmT,dt)){MovieRepo_Delete(confirmIdx);flash="Movie deleted.";flashT=2.5f;flashC=C_ACCENT;confirmIdx=-1;}
                }
                Rectangle ab={SW-178.f,(float)(SH-58),150.f,40.f};
                bool ah=CheckCollisionPointRec(GetMousePosition(),ab);
                DrawRectangleRec(ab,CA(C_GOLD,ah?0.40f:0.20f));DrawRectangleLinesEx(ab,1.f,CA(C_GOLD,0.70f));
                float alw=MeasureTextEx(fUIBold,"+ ADD MOVIE",11.f,2).x;DrawTextEx(fUIBold,"+ ADD MOVIE",{ab.x+(150.f-alw)*0.5f,ab.y+12.f},11.f,2,C_GOLD2);
                if(ah&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){addFormOpen=true;selTheme=0;initFrm();fTitle2.buf=fGenre.buf=fDir.buf=fCast.buf=fDur.buf=fYear.buf=fRating.buf=fPrice.buf=fDesc.buf="";}
            } else {
                // Add form overlay
                auto uf=[&](Field& f){FieldUpdate(f,dt);if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))f.active=CheckCollisionPointRec(GetMousePosition(),f.rect);};
                uf(fTitle2);uf(fGenre);uf(fDir);uf(fCast);uf(fDur);uf(fYear);uf(fRating);uf(fPrice);uf(fDesc);
                DrawRectangle(0,0,SW,SH,CA(C_BG,0.90f));
                DrawRectangleRounded({SW*0.5f-230.f,90.f,460.f,(float)(SH-110)},0.04f,8,CA(C_PANEL,0.97f));
                DrawRectangleLinesEx({SW*0.5f-230.f,90.f,460.f,(float)(SH-110)},1.5f,CA(C_GOLDDIM,0.45f));
                DrawTextC(fUIBold,"ADD NEW MOVIE",SW*0.5f,96.f,13.f,2,C_GOLD2);
                FieldDraw(fTitle2);FieldDraw(fGenre);FieldDraw(fDir);FieldDraw(fCast);FieldDraw(fDur);FieldDraw(fYear);FieldDraw(fRating);FieldDraw(fPrice);FieldDraw(fDesc);
                // Colour theme picker
                float thY=fDesc.rect.y+fDesc.rect.height+12.f;
                DrawTextEx(fUI,"COLOUR THEME:",{SW*0.5f-210.f,thY},9.5f,2,CA(C_GOLD,0.60f));thY+=16.f;
                for(int i=0;i<8;i++){
                    float tx=SW*0.5f-210.f+i*54.f;bool ts=(selTheme==i);
                    const ColourTheme& th2=COLOUR_THEMES[i];
                    Color c1={(unsigned char)th2.pA_r,(unsigned char)th2.pA_g,(unsigned char)th2.pA_b,255};
                    Color c2={(unsigned char)th2.ac_r,(unsigned char)th2.ac_g,(unsigned char)th2.ac_b,255};
                    DrawRectangleGradientH((int)tx,(int)thY,50,20,c1,c2);
                    DrawRectangleLinesEx({tx,thY,50.f,20.f},ts?2.f:0.8f,ts?C_GOLD2:CA(C_GREY,0.35f));
                    DrawTextEx(fUI,th2.name,{tx+1.f,thY+7.f},5.5f,1,CA(C_WHITE,0.75f));
                    if(CheckCollisionPointRec(GetMousePosition(),{tx,thY,50.f,20.f})&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selTheme=i;
                }
                thY+=28.f;
                float fw2=SW*0.5f-210.f;
                Rectangle sv2={fw2,thY,210.f,40.f},cn2={fw2+214.f,thY,210.f,40.f};
                bool sh=CheckCollisionPointRec(GetMousePosition(),sv2),ch=CheckCollisionPointRec(GetMousePosition(),cn2);
                DrawRectangleRec(sv2,CA(C_GOLD,sh?0.40f:0.20f));DrawRectangleLinesEx(sv2,1.f,CA(C_GOLD,0.70f));
                float slw=MeasureTextEx(fUIBold,"SAVE MOVIE",11.f,2).x;DrawTextEx(fUIBold,"SAVE MOVIE",{sv2.x+(210.f-slw)*0.5f,sv2.y+13.f},11.f,2,C_GOLD2);
                DrawRectangleRec(cn2,CA(C_PANEL,ch?0.90f:0.65f));DrawRectangleLinesEx(cn2,1.f,CA(C_GREY,0.40f));
                float clw2=MeasureTextEx(fUIBold,"CANCEL",11.f,2).x;DrawTextEx(fUIBold,"CANCEL",{cn2.x+(210.f-clw2)*0.5f,cn2.y+13.f},11.f,2,CA(C_GREYLT,0.75f));
                if(ch&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) addFormOpen=false;
                if(IsKeyPressed(KEY_ESCAPE)) addFormOpen=false;
                if(sh&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                    if(fTitle2.buf.empty()){flash="Title required!";flashT=2.f;flashC=C_RED;}
                    else{
                        MovieEntry ne;const ColourTheme& th3=COLOUR_THEMES[selTheme];
                        snprintf(ne.title,sizeof(ne.title),"%s",fTitle2.buf.c_str());
                        snprintf(ne.genre,sizeof(ne.genre),"%s",fGenre.buf.c_str());
                        snprintf(ne.director,sizeof(ne.director),"%s",fDir.buf.c_str());
                        snprintf(ne.cast,sizeof(ne.cast),"%s",fCast.buf.c_str());
                        snprintf(ne.duration,sizeof(ne.duration),"%s",fDur.buf.c_str());
                        snprintf(ne.year,sizeof(ne.year),"%s",fYear.buf.c_str());
                        snprintf(ne.desc,sizeof(ne.desc),"%s",fDesc.buf.c_str());
                        try{ne.rating=std::stof(fRating.buf);}catch(...){ne.rating=7.f;}
                        try{ne.price=std::stof(fPrice.buf);}catch(...){ne.price=10.f;}
                        ne.pA_r=th3.pA_r;ne.pA_g=th3.pA_g;ne.pA_b=th3.pA_b;
                        ne.pB_r=th3.pB_r;ne.pB_g=th3.pB_g;ne.pB_b=th3.pB_b;
                        ne.ac_r=th3.ac_r;ne.ac_g=th3.ac_g;ne.ac_b=th3.ac_b;
                        MovieRepo_Add(ne);flash="Movie added!";flashT=2.5f;flashC=C_GREEN;addFormOpen=false;
                    }
                }
            }
        }

        // ══════ STATISTICS TAB ════════════════════════════════
        else if(curTab==AT_STATS){
            AppStats s=StatsSvc_Compute();
            float sy2=CY+12.f;float sw5=(float)(SW-100)/4.f;
            struct St{const char* lbl;char val[32];Color col;};
            St stats2[4]; 
            snprintf(stats2[0].val,32,"%d",s.totalUsers);stats2[0].lbl="USERS";stats2[0].col=C_ACCENT;
            snprintf(stats2[1].val,32,"%d",s.totalMovies);stats2[1].lbl="MOVIES";stats2[1].col=C_GOLD;
            snprintf(stats2[2].val,32,"%d",s.totalBookings);stats2[2].lbl="BOOKINGS";stats2[2].col=C_GOLD2;
            snprintf(stats2[3].val,32,"%.2f BGN",s.totalRevenue);stats2[3].lbl="REVENUE";stats2[3].col=C_GREEN;
            for(int i=0;i<4;i++){
                float bx=50.f+i*sw5;
                DrawRectangle((int)bx,(int)sy2,(int)(sw5-8),56,CA(C_PANEL,0.75f));
                DrawRectangleLinesEx({bx,sy2,sw5-8.f,56.f},0.8f,CA(stats2[i].col,0.40f));
                DrawRectangle((int)bx,(int)sy2,(int)(sw5-8),3,CA(stats2[i].col,0.70f));
                DrawTextC(fUIBold,stats2[i].val,bx+(sw5-8.f)*0.5f,sy2+8.f,15.f,2,stats2[i].col);
                DrawTextC(fUI,stats2[i].lbl,bx+(sw5-8.f)*0.5f,sy2+36.f,8.5f,1,CA(C_GREYLT,0.55f));
            }
            sy2+=72.f;
            // Bar chart
            DrawTextEx(fUIBold,"BOOKINGS BY MOVIE",{50.f,sy2},10.5f,2,CA(C_GOLD,0.65f));sy2+=20.f;
            int maxB=1;for(auto& ms:s.byMovie) if(ms.bookings>maxB) maxB=ms.bookings;
            float barAreaW=(float)(SW-100),barW=std::min(60.f,barAreaW/(float)std::max(1,(int)s.byMovie.size())-4.f);
            float chartH=std::min(200.f,(float)(SH-sy2-100));float chartX=50.f;
            DrawRectangle((int)chartX,(int)sy2,(int)barAreaW,(int)chartH,CA(C_PANEL,0.40f));
            for(int i=0;i<(int)s.byMovie.size();i++){
                float bx=chartX+2.f+i*(barW+4.f),bh=(float)s.byMovie[i].bookings/(float)maxB*chartH;
                float by=sy2+chartH-bh;
                if(i<MovieRepo_Count()){const MovieEntry& e=MovieRepo_Get(i);Color ac=Ac5(e);
                    DrawRectangle((int)bx,(int)by,(int)barW,(int)bh,CA(ac,0.70f));
                    DrawRectangleLinesEx({bx,by,barW,bh},0.8f,CA(ac,0.90f));
                }
                if(s.byMovie[i].bookings>0){char bv[8];snprintf(bv,sizeof(bv),"%d",s.byMovie[i].bookings);float bvw=MeasureTextEx(fUI,bv,8.f,1).x;DrawTextEx(fUI,bv,{bx+(barW-bvw)*0.5f,by-12.f},8.f,1,CA(C_WHITE,0.70f));}
                // Truncated movie name below bar
                std::string mn=s.byMovie[i].title;if(mn.size()>8) mn=mn.substr(0,6)+"..";
                float nlw=MeasureTextEx(fUI,mn.c_str(),7.f,1).x;DrawTextEx(fUI,mn.c_str(),{bx+(barW-nlw)*0.5f,sy2+chartH+4.f},7.f,1,CA(C_GREYLT,0.50f));
            }
            sy2+=chartH+22.f;
            // Revenue table
            DrawTextEx(fUIBold,"REVENUE BREAKDOWN",{50.f,sy2},10.5f,2,CA(C_GOLD,0.65f));sy2+=18.f;
            BeginScissorMode(28,(int)sy2,SW-56,(int)(SH-sy2-14));
            for(int i=0;i<(int)s.byMovie.size()&&sy2<SH-20.f;i++){
                float ry=sy2+i*28.f;
                bool h=CheckCollisionPointRec(GetMousePosition(),{28.f,ry,(float)(SW-56),26.f});
                DrawRectangle(28,(int)ry,SW-56,26,CA(C_PANEL,h?0.75f:0.45f+(i%2)*0.08f));
                if(i<MovieRepo_Count()){const MovieEntry& e=MovieRepo_Get(i);Color ac=Ac5(e);DrawRectangle(28,(int)ry,4,26,CA(ac,0.55f));}
                DrawTextEx(fUI,TruncText(s.byMovie[i].title.c_str(),fUI,10.f,1,400.f).c_str(),{38.f,ry+7.f},10.f,1,CA(C_GREYLT,0.80f));
                char rb2[16];snprintf(rb2,sizeof(rb2),"%d",s.byMovie[i].bookings);
                DrawTextEx(fUI,rb2,{SW*0.6f,ry+7.f},10.f,1,CA(C_GOLD,0.70f));
                char rv[24];snprintf(rv,sizeof(rv),"%.2f BGN",s.byMovie[i].revenue);
                float rvw=MeasureTextEx(fUI,rv,10.f,1).x;DrawTextEx(fUI,rv,{(float)(SW-50)-rvw,ry+7.f},10.f,1,CA(C_GREEN,0.80f));
            }
            EndScissorMode();
        }

        // Flash
        if(flashT>0.f){float a=std::min(1.f,flashT/0.4f);float fw3=MeasureTextEx(fUIBold,flash.c_str(),11.f,1).x;
            DrawRectangle((int)(SW*0.5f-fw3*0.5f-14),(int)(SH-46),(int)(fw3+28),30,CA(C_PANEL,a*0.95f));
            DrawRectangleLinesEx({SW*0.5f-fw3*0.5f-14.f,SH-46.f,fw3+28.f,30.f},1.f,CA(flashC,a*0.60f));
            DrawTextEx(fUIBold,flash.c_str(),{SW*0.5f-fw3*0.5f,SH-39.f},11.f,1,CA(flashC,a));}

        DrawScanlines();
        if(fadeIn<1.f) DrawRectangle(0,0,SW,SH,CA(C_BG,1.f-fadeIn));
        DrawTextC(fUI,"(c) 2025 Movix",SW*0.5f,SH-14.f,9.f,1,CA(C_GREY,0.25f));
        EndDrawing();
    }
}
