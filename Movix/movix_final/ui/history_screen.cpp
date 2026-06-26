#include "theme.h"
#include "screens.h"
#include "../bll/session.h"
#include "../bll/booking_service.h"
#include "../dal/movie_repo.h"
#include "../dal/reservation_repo.h"
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <map>
#include <vector>

static Color Ac4(const MovieEntry& e){return {(unsigned char)e.ac_r,(unsigned char)e.ac_g,(unsigned char)e.ac_b,255};}

void RunHistoryScreen(){
    float fadeIn=0.f,scrollY=0.f,scrollTarget=0.f;
    Button backBtn; BtnInit(backBtn,{36.f,22.f,140.f,36.f},"< BACK");
    std::string flash; float flashT=0.f; Color flashC=C_GREEN;

    while(!WindowShouldClose()){
        float dt=GetFrameTime(),t=GetTime();
        fadeIn=std::min(1.f,fadeIn+dt*2.f);
        UpdateParticles(dt);
        if(flashT>0.f) flashT-=dt;
        if(BtnUpdate(backBtn,dt)||IsKeyPressed(KEY_ESCAPE)) return;

        // Load bookings grouped by movie
        auto myRes=ResRepo_ForUser(gCurrentUser);
        std::map<int,std::vector<Reservation>> byMovie;
        for(auto& r:myRes) byMovie[r.movieIdx].push_back(r);

        // Scroll
        float totalH=(float)byMovie.size()*100.f+(float)myRes.size()*30.f+40.f;
        float contentY=90.f,contentH=SH-contentY-20.f;
        float maxScroll=std::max(0.f,totalH-contentH);
        scrollTarget-=GetMouseWheelMove()*50.f;scrollTarget=std::clamp(scrollTarget,0.f,maxScroll);
        scrollY+=(scrollTarget-scrollY)*std::min(1.f,dt*12.f);

        BeginDrawing();ClearBackground(C_BG);DrawParticles();
        DrawFilm(0,0,SH,t,false);DrawFilm(SW-28,0,SH,t,true);
        DrawRectangle(28,0,SW-56,80,CA(C_PANEL,0.97f));DrawRectangle(28,78,SW-56,2,CA(C_GOLDDIM,0.5f));
        DrawTextC(fTitle,"MOVIX",SW*0.5f,12,36,4,C_GOLD2);
        DrawTextEx(fUI,"MY BOOKING HISTORY",{50.f,30.f},11.f,3,CA(C_GOLD,0.60f));
        char tot[40];snprintf(tot,sizeof(tot),"%d reservation%s total",(int)myRes.size(),(int)myRes.size()==1?"":"s");
        float tw=MeasureTextEx(fUI,tot,9.5f,1).x;DrawTextEx(fUI,tot,{(float)(SW-tw-40),(float)46},9.5f,1,CA(C_GREYLT,0.45f));
        BtnDraw(backBtn);

        if(myRes.empty()){
            DrawTextC(fUIBold,"No bookings yet",SW*0.5f,SH*0.5f-20.f,16.f,2,CA(C_GREY,0.45f));
            DrawTextC(fUI,"Reserve seats from the movie catalogue",SW*0.5f,SH*0.5f+14.f,11.f,1,CA(C_GREY,0.30f));
        } else {
            BeginScissorMode(28,(int)contentY,SW-56,(int)contentH);
            float y=contentY+10.f-scrollY;
            for(auto& [midx,seats]:byMovie){
                if(midx>=MovieRepo_Count()) continue;
                const MovieEntry& e=MovieRepo_Get(midx);Color ac=Ac4(e);
                // Movie header row
                if(y+80.f>contentY&&y<contentY+contentH){
                    DrawRectangle(28,(int)y,SW-56,40,CA(C_PANEL,0.80f));
                    DrawRectangle(28,(int)y,6,40,CA(ac,0.70f));
                    DrawTextEx(fUIBold,TruncText(e.title,fUIBold,12.f,1,500.f).c_str(),{44.f,y+12.f},12.f,1,C_GOLD2);
                    char mc[32];snprintf(mc,sizeof(mc),"%d seat%s",(int)seats.size(),(int)seats.size()==1?"":"s");
                    float mw=MeasureTextEx(fUI,mc,10.f,1).x;
                    DrawTextEx(fUI,mc,{(float)(SW-50)-mw,y+13.f},10.f,1,CA(ac,0.75f));
                }
                y+=44.f;
                // Seat rows
                for(auto& r:seats){
                    if(y+28.f>contentY&&y<contentY+contentH){
                        bool hov=CheckCollisionPointRec(GetMousePosition(),{30.f,y,(float)(SW-60),26.f});
                        DrawRectangle(30,(int)y,SW-60,26,CA(C_PANEL,hov?0.80f:0.50f));
                        char slbl[8];snprintf(slbl,sizeof(slbl),"%c%d",'A'+r.row,r.col+1);
                        DrawCircle(55,(int)(y+13),4,CA(ac,0.75f));
                        DrawTextEx(fUIBold,slbl,{66.f,y+7.f},11.f,1,C_WHITE);
                        const char* tier=BookSvc_RowTier(r.row);
                        DrawTextEx(fUI,tier,{100.f,y+8.f},9.f,1,CA(C_GREYLT,0.50f));
                        char pr[16];snprintf(pr,sizeof(pr),"%.2f BGN",BookSvc_SeatPrice(r.row,e.price));
                        float pw5=MeasureTextEx(fUI,pr,10.f,1).x;
                        DrawTextEx(fUI,pr,{(float)(SW-150)-pw5,y+8.f},10.f,1,CA(C_GOLD,0.80f));
                        // Cancel button
                        Rectangle cb={SW-138.f,y+3.f,100.f,20.f};
                        bool ch=hov&&CheckCollisionPointRec(GetMousePosition(),cb);
                        DrawRectangleRec(cb,CA(C_RED,ch?0.60f:0.28f));
                        DrawRectangleLinesEx(cb,0.8f,CA(C_RED,0.70f));
                        float clw=MeasureTextEx(fUI,"CANCEL",8.f,1).x;
                        DrawTextEx(fUI,"CANCEL",{cb.x+(cb.width-clw)*0.5f,cb.y+6.f},8.f,1,CA(C_WHITE,0.90f));
                        if(ch&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                            BookSvc_Cancel(midx,r.row,r.col,gCurrentUser);
                            flash="Reservation cancelled.";flashT=2.5f;flashC=C_ACCENT;
                        }
                    }
                    y+=29.f;
                }
                y+=8.f; // gap between movies
            }
            EndScissorMode();
        }

        // Flash
        if(flashT>0.f){float a=std::min(1.f,flashT/0.4f);float fw=MeasureTextEx(fUIBold,flash.c_str(),11.f,1).x;
            DrawRectangle((int)(SW*0.5f-fw*0.5f-14),(int)(SH-50),(int)(fw+28),30,CA(C_PANEL,a*0.95f));
            DrawRectangleLinesEx({SW*0.5f-fw*0.5f-14,SH-50.f,fw+28.f,30.f},1.f,CA(flashC,a*0.60f));
            DrawTextEx(fUIBold,flash.c_str(),{SW*0.5f-fw*0.5f,SH-43.f},11.f,1,CA(flashC,a));}

        DrawScanlines();
        if(fadeIn<1.f) DrawRectangle(0,0,SW,SH,CA(C_BG,1.f-fadeIn));
        DrawTextC(fUI,"(c) 2025 Movix",SW*0.5f,SH-14.f,9.f,1,CA(C_GREY,0.25f));
        EndDrawing();
    }
}
