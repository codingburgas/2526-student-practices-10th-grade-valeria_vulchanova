#include "theme.h"
#include "screens.h"
#include "../bll/session.h"
#include "../bll/booking_service.h"
#include "../dal/movie_repo.h"
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <set>
#include <vector>

static Color Ac2(const MovieEntry& e){return {(unsigned char)e.ac_r,(unsigned char)e.ac_g,(unsigned char)e.ac_b,255};}
static Color TC(int row){if(row<=1)return{150,150,150,255};if(row>=6)return C_GOLD2;return C_WHITE;}

void RunSeatScreen(int movieIdx){
    if(MovieRepo_Count()==0) return;
    const MovieEntry& me=MovieRepo_Get(movieIdx);
    Color ac=Ac2(me);

    const float SW2=40.f,SH2=34.f,SGAP=8.f,AIW=24.f;
    const float TOT=(float)HALL_COLS*(SW2+SGAP)+AIW-SGAP;
    const float SX=(SW-TOT)*0.5f,SY=165.f,SCRH=SY-45.f;
    const float SCRW=TOT*0.72f,SCRX=SX+(TOT-SCRW)*0.5f;
    const float IFX=SX+TOT+30.f,IFW=SW-IFX-28.f;
    const float RH=SH2+SGAP;

    std::set<int> occupied=BookSvc_OccupiedSeats(movieIdx);
    std::set<int> mySeats;
    for(auto& r:ResRepo_ForUser(gCurrentUser)) if(r.movieIdx==movieIdx) mySeats.insert(r.row*HALL_COLS+r.col);

    bool sel[HALL_ROWS*HALL_COLS]={};
    float hov[HALL_ROWS*HALL_COLS]={};
    float fadeIn=0.f,confirmPopT=0.f;
    bool confirmed=false;

    Button confirmBtn,backBtn;
    BtnInit(confirmBtn,{SW*0.5f-140.f,SH-64.f,280.f,48.f},"CONFIRM BOOKING");
    BtnInit(backBtn,{30.f,SH-64.f,120.f,48.f},"< BACK");

    while(!WindowShouldClose()){
        float dt=GetFrameTime(),t=GetTime();
        fadeIn=std::min(1.f,fadeIn+dt*2.f);
        UpdateParticles(dt);
        if(confirmPopT>0.f) confirmPopT-=dt;

        Vector2 mouse=GetMousePosition();
        int selCnt=0;
        for(int i=0;i<HALL_ROWS*HALL_COLS;i++){
            if(sel[i]) selCnt++;
            int c=i%HALL_COLS,r=i/HALL_COLS;
            float ai=(c>=5)?AIW:0.f,sx2=SX+c*(SW2+SGAP)+ai,sy2=SY+r*RH;
            bool taken=occupied.count(i)||mySeats.count(i);
            bool h=!taken&&CheckCollisionPointRec(mouse,{sx2,sy2,SW2,SH2});
            hov[i]+=dt*(h?9.f:-9.f);hov[i]=std::clamp(hov[i],0.f,1.f);
            if(h&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)&&confirmPopT<=0.f) sel[i]=!sel[i];
        }
        bool doConf=BtnUpdate(confirmBtn,dt);
        bool doBack=BtnUpdate(backBtn,dt)||IsKeyPressed(KEY_ESCAPE);
        if(doConf&&selCnt>0&&confirmPopT<=0.f){
            for(int i=0;i<HALL_ROWS*HALL_COLS;i++) if(sel[i]){
                BookSvc_Reserve(movieIdx,i/HALL_COLS,i%HALL_COLS,gCurrentUser);
                occupied.insert(i);mySeats.insert(i);
            }
            memset(sel,0,sizeof(sel));confirmPopT=3.f;confirmed=true;
        }
        if(doBack) return;
        if(confirmed&&confirmPopT<=0.f) return;

        BeginDrawing();ClearBackground(C_BG);DrawParticles();
        DrawFilm(0,0,SH,t,false);DrawFilm(SW-28,0,SH,t,true);
        DrawRectangle(28,0,SW-56,80,CA(C_PANEL,0.97f));DrawRectangle(28,78,SW-56,2,CA(C_GOLDDIM,0.5f));
        DrawTextC(fTitle,"MOVIX",SW*0.5f,12,36,4,C_GOLD2);
        char hdr[128];snprintf(hdr,sizeof(hdr),"SEATS  -  %s",TruncText(me.title,fUI,11.f,2,SW-400.f).c_str());
        DrawTextEx(fUI,hdr,{50.f,30.f},11.f,2,CA(C_GOLD,0.60f));

        // Screen glow
        for(int gi=2;gi>=0;gi--){float sp=10.f+gi*14.f;DrawRectangleGradientV((int)(SCRX-sp),(int)(SCRH-sp*0.4f),(int)(SCRW+sp*2),(int)(sp+12),CA(ac,0.f),CA(ac,(0.10f-gi*0.03f)+sinf(t*1.1f)*0.02f));}
        DrawRectangleRounded({SCRX,SCRH,SCRW,11.f},0.5f,8,CA(ac,0.55f+sinf(t*1.2f)*0.07f));
        DrawRectangleRoundedLines({SCRX,SCRH,SCRW,11.f},0.5f,8,1.3f,CA(ac,0.90f));
        float sw3=MeasureTextEx(fUI,"SCREEN",9.f,2).x;
        DrawTextEx(fUI,"SCREEN",{SCRX+(SCRW-sw3)*0.5f,SCRH+1.f},9.f,2,CA(C_BG,0.9f));

        // Seats
        for(int i=0;i<HALL_ROWS*HALL_COLS;i++){
            int c=i%HALL_COLS,r=i/HALL_COLS;
            float ai=(c>=5)?AIW:0.f,sx2=SX+c*(SW2+SGAP)+ai,sy2=SY+r*RH;
            bool taken=occupied.count(i)>0,mine=mySeats.count(i)>0,isSel=sel[i];float hv=hov[i];
            Color fillC,bordC;
            if(mine){fillC=CA(ac,0.35f);bordC=CA(ac,0.80f);}
            else if(taken){fillC={25,18,18,255};bordC=CA(C_GREY,0.18f);}
            else if(isSel){fillC=CA(ac,0.65f+hv*0.15f);bordC=ac;}
            else{Color tc=TC(r);fillC=CA(C_PANEL,0.60f+hv*0.25f);bordC=CA(tc,0.25f+hv*0.40f);}
            float bH=SH2*0.3f;
            DrawRectangleRounded({sx2+2.f,sy2,SW2-4.f,bH+2.f},0.55f,4,fillC);
            DrawRectangleRounded({sx2,sy2+bH,SW2,SH2-bH},0.30f,4,fillC);
            DrawRectangleRoundedLines({sx2,sy2,SW2,SH2},0.25f,4,0.8f,bordC);
            if(!taken){char lb[6];snprintf(lb,sizeof(lb),"%c%d",'A'+r,c+1);float lw=MeasureTextEx(fUI,lb,7.f,1).x;DrawTextEx(fUI,lb,{sx2+(SW2-lw)*0.5f,sy2+SH2*0.35f},7.f,1,isSel?CA(C_BG,0.85f):CA(C_GREY,0.45f));}
            else if(!mine){float cx3=sx2+SW2*0.5f,cy3=sy2+SH2*0.6f;DrawLine((int)(cx3-4),(int)(cy3-4),(int)(cx3+4),(int)(cy3+4),CA(C_GREY,0.4f));DrawLine((int)(cx3+4),(int)(cy3-4),(int)(cx3-4),(int)(cy3+4),CA(C_GREY,0.4f));}
        }
        // Row labels
        for(int r=0;r<HALL_ROWS;r++){char rl[2]={(char)('A'+r),0};DrawTextEx(fUIBold,rl,{SX-24.f,SY+r*RH+(SH2-13.f)*0.5f},13.f,1,CA(TC(r),0.65f));}

        // Legend
        float lY=SY+HALL_ROWS*RH+10.f;
        auto LI=[&](float x,Color fc,Color bc,const char* lb){DrawRectangleRounded({x,lY,18.f,12.f},0.35f,4,fc);DrawRectangleRoundedLines({x,lY,18.f,12.f},0.35f,4,0.8f,bc);DrawTextEx(fUI,lb,{x+22.f,lY+1.f},8.5f,1,CA(C_GREYLT,0.55f));};
        LI(SX,CA(C_PANEL,0.60f),CA(C_GOLDDIM,0.50f),"Available");
        LI(SX+120.f,CA(ac,0.65f),ac,"Selected");
        LI(SX+230.f,{25,18,18,255},CA(C_GREY,0.25f),"Taken");
        LI(SX+320.f,CA(ac,0.35f),CA(ac,0.80f),"Yours");
        DrawTextEx(fUI,"Front A-B: -20%  STD C-F: base  VIP G-H: +25%",{SX,lY+15.f},7.5f,1,CA(C_GREY,0.38f));

        // Info panel
        if(IFW>60.f){
            float iy=SY;
            DrawTextEx(fUIBold,me.title,{IFX,iy},11.f,2,C_GOLD2);iy+=18.f;
            DrawTextEx(fUI,me.genre,{IFX,iy},9.f,1,CA(C_GREYLT,0.60f));iy+=14.f;
            DrawTextEx(fUI,me.duration,{IFX,iy},9.f,1,CA(C_GREYLT,0.50f));iy+=20.f;
            int selCnt2=0;for(int i=0;i<HALL_ROWS*HALL_COLS;i++) if(sel[i]) selCnt2++;
            char sb[40];snprintf(sb,sizeof(sb),"Selected: %d seat%s",selCnt2,selCnt2==1?"":"s");
            DrawTextEx(fUIBold,sb,{IFX,iy},11.f,1,selCnt2?C_GOLD2:CA(C_GREY,0.45f));iy+=18.f;
            if(selCnt2>0){
                float total=0;for(int i=0;i<HALL_ROWS*HALL_COLS;i++) if(sel[i]) total+=BookSvc_SeatPrice(i/HALL_COLS,me.price);
                char tot[40];snprintf(tot,sizeof(tot),"Total: %.2f BGN",total);
                DrawRectangle((int)IFX,(int)iy,(int)IFW,28,CA(C_GOLD,0.10f));DrawRectangleLinesEx({IFX,iy,(float)IFW,28.f},1.f,CA(C_GOLD,0.35f));
                DrawTextEx(fUIBold,tot,{IFX+8.f,iy+7.f},12.f,2,C_GOLD2);iy+=34.f;
                for(int i=0;i<HALL_ROWS*HALL_COLS&&iy<SH-90.f;i++) if(sel[i]){
                    int r=i/HALL_COLS,c=i%HALL_COLS;char s[12],p[12];
                    snprintf(s,sizeof(s),"%c%d",'A'+r,c+1);snprintf(p,sizeof(p),"%.2f",BookSvc_SeatPrice(r,me.price));
                    DrawCircle((int)(IFX+8.f),(int)(iy+7.f),4,CA(TC(r),0.80f));
                    DrawTextEx(fUI,s,{IFX+18.f,iy},9.5f,1,CA(C_GREYLT,0.85f));
                    float pw4=MeasureTextEx(fUI,p,9.5f,1).x;DrawTextEx(fUI,p,{IFX+IFW-pw4,iy},9.5f,1,CA(C_GOLD,0.80f));iy+=18.f;
                }
            }
        }

        BtnDraw(backBtn);
        if(selCnt>0) BtnDraw(confirmBtn);
        else{Button tmp=confirmBtn;tmp.text="CONFIRM BOOKING";BtnDrawDisabled(tmp);}

        if(confirmPopT>0.f){
            float a=std::min(1.f,confirmPopT/0.35f)*std::min(1.f,confirmPopT);
            DrawRectangle(0,0,SW,SH,CA(C_BG,a*0.80f));
            float pw5=420.f,ph5=100.f,px5=(SW-pw5)*0.5f,py5=(SH-ph5)*0.45f;
            DrawRectangleRounded({px5,py5,pw5,ph5},0.08f,8,CA(C_PANEL,0.98f));
            DrawRectangleLinesEx({px5,py5,pw5,ph5},2.f,CA(ac,a));
            DrawRectangleRounded({px5,py5,pw5,5.f},0.5f,8,CA(ac,a*0.85f));
            DrawTextC(fUIBold,"Seats booked successfully!",px5+pw5*0.5f,py5+22.f,13.f,2,CA(ac,a));
            DrawTextC(fUI,"Returning...",px5+pw5*0.5f,py5+ph5-20.f,9.f,1,CA(C_GREY,a*0.45f));
        }
        DrawScanlines();
        if(fadeIn<1.f) DrawRectangle(0,0,SW,SH,CA(C_BG,1.f-fadeIn));
        DrawTextC(fUI,"(c) 2025 Movix",SW*0.5f,SH-14.f,9.f,1,CA(C_GREY,0.25f));
        EndDrawing();
    }
}
