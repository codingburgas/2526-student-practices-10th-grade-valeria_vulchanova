#include "theme.h"
#include "screens.h"
#include "../bll/session.h"
#include "../bll/auth_service.h"
#include "../dal/movie_repo.h"
#include "../dal/reservation_repo.h"
#include "../dal/rating_repo.h"
#include <cmath>
#include <set>
#include "../bll/booking_service.h"
#include <algorithm>
#include <cstdio>
#include <map>

static Color Ac3(const MovieEntry& e){return {(unsigned char)e.ac_r,(unsigned char)e.ac_g,(unsigned char)e.ac_b,255};}

void RunProfileScreen(){
    float fadeIn=0.f;
    Field oldPF,newPF,cnfPF;
    FieldInit(oldPF,{SW*0.5f-160.f,260.f,320.f,46.f},"CURRENT PASSWORD",true);
    FieldInit(newPF,{SW*0.5f-160.f,330.f,320.f,46.f},"NEW PASSWORD",true);
    FieldInit(cnfPF,{SW*0.5f-160.f,400.f,320.f,46.f},"CONFIRM NEW PASSWORD",true);
    Button saveBtn,backBtn;
    BtnInit(saveBtn,{SW*0.5f-130.f,462.f,260.f,44.f},"SAVE PASSWORD");
    BtnInit(backBtn,{36.f,22.f,140.f,36.f},"< BACK");
    std::string msg; Color mc=C_RED; float mAlpha=0.f,mT=0.f;

    while(!WindowShouldClose()){
        float dt=GetFrameTime(),t=GetTime();
        fadeIn=std::min(1.f,fadeIn+dt*2.f);
        UpdateParticles(dt);

        FieldUpdate(oldPF,dt);FieldUpdate(newPF,dt);FieldUpdate(cnfPF,dt);
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            oldPF.active=CheckCollisionPointRec(GetMousePosition(),oldPF.rect);
            newPF.active=CheckCollisionPointRec(GetMousePosition(),newPF.rect);
            cnfPF.active=CheckCollisionPointRec(GetMousePosition(),cnfPF.rect);
        }
        if(IsKeyPressed(KEY_TAB)){
            if(oldPF.active){oldPF.active=false;newPF.active=true;}
            else if(newPF.active){newPF.active=false;cnfPF.active=true;}
            else{cnfPF.active=false;oldPF.active=true;}
        }
        if(BtnUpdate(backBtn,dt)||IsKeyPressed(KEY_ESCAPE)) return;
        if(BtnUpdate(saveBtn,dt)){
            if(newPF.buf!=cnfPF.buf){msg="New passwords don't match.";mc=C_RED;mAlpha=1.f;mT=3.f;}
            else if(newPF.buf.size()<4){msg="Min 4 characters.";mc=C_RED;mAlpha=1.f;mT=3.f;}
            else if(!AuthChangePassword(gCurrentUser,oldPF.buf,newPF.buf)){msg="Current password is wrong.";mc=C_RED;mAlpha=1.f;mT=3.f;}
            else{msg="Password changed!";mc=C_GREEN;mAlpha=1.f;mT=3.f;oldPF.buf=newPF.buf=cnfPF.buf="";}
        }
        if(mT>0){mT-=dt;mAlpha=std::min(1.f,mT/0.4f);}

        // Stats
        auto myRes=ResRepo_ForUser(gCurrentUser);
        auto myRatings=RatingRepo_LoadAll();
        float spent=0.f;std::set<int> ratedSet;
        std::map<int,int> perMovie;
        for(auto& r:myRes){
            if(r.movieIdx<MovieRepo_Count()){
                const auto& e=MovieRepo_Get(r.movieIdx);
                spent+=BookSvc_SeatPrice(r.row,e.price);
                perMovie[r.movieIdx]++;
            }
        }
        int ratedCount=0;
        for(auto& r:myRatings) if(r.username==gCurrentUser){ratedCount++;ratedSet.insert(r.movieId);}

        BeginDrawing();ClearBackground(C_BG);DrawParticles();
        DrawFilm(0,0,SH,t,false);DrawFilm(SW-28,0,SH,t,true);
        DrawRectangle(28,0,SW-56,80,CA(C_PANEL,0.97f));DrawRectangle(28,78,SW-56,2,CA(C_GOLDDIM,0.5f));
        DrawTextC(fTitle,"MOVIX",SW*0.5f,12,36,4,C_GOLD2);
        DrawTextEx(fUI,"MY PROFILE",{50.f,30.f},11.f,3,CA(C_GOLD,0.60f));
        BtnDraw(backBtn);

        float cx=SW*0.5f;
        // Avatar circle
        DrawCircle((int)cx,138,36,CA(C_GOLDDIM,0.35f));DrawCircleLines((int)cx,138,36,CA(C_GOLD,0.55f));
        std::string ini=gCurrentUser.empty()?"?":std::string(1,toupper(gCurrentUser[0]));
        DrawTextC(fUIBold,ini.c_str(),cx,118,28,2,C_GOLD2);
        DrawTextC(fUIBold,gCurrentUser.c_str(),cx,182.f,16.f,2,C_WHITE);
        DrawTextC(fUI,gIsAdmin?"Administrator":"Member",cx,203.f,10.5f,1,CA(C_GREYLT,0.55f));

        // Stats row
        float sy2=230.f;
        struct Stat{const char* label;char val[32];};
        char sb[32],rb[32],bb[32];
        snprintf(sb,sizeof(sb),"%.2f BGN",spent);
        snprintf(rb,sizeof(rb),"%d",ratedCount);
        snprintf(bb,sizeof(bb),"%d",(int)myRes.size());
        Stat stats[]={{"BOOKINGS",""}, {"SPENT",""}, {"RATED",""}};
        snprintf(stats[0].val,32,"%d",(int)myRes.size());
        snprintf(stats[1].val,32,"%.2f BGN",spent);
        snprintf(stats[2].val,32,"%d",ratedCount);
        float sw4=(float)(SW-100)/3.f;
        for(int i=0;i<3;i++){
            float bx=50.f+i*sw4;
            DrawRectangle((int)bx,(int)sy2,(int)(sw4-10),52,CA(C_PANEL,0.75f));
            DrawRectangleLinesEx({bx,sy2,sw4-10.f,52.f},0.8f,CA(C_GOLDDIM,0.35f));
            DrawTextC(fUIBold,stats[i].val,bx+(sw4-10.f)*0.5f,sy2+6.f,16.f,2,C_GOLD2);
            DrawTextC(fUI,stats[i].label,bx+(sw4-10.f)*0.5f,sy2+32.f,9.f,1,CA(C_GREYLT,0.55f));
        }

        // Change password section
        DrawRectangle(28,290,SW-56,1,CA(C_GOLDDIM,0.25f));
        // But wait — fields are positioned at 260 etc which overlaps with stats (sy2=230+52=282)
        // Let me adjust: draw section title
        float fieldTop=300.f;
        // Override field rects
        oldPF.rect={cx-160.f,fieldTop,320.f,46.f};
        newPF.rect={cx-160.f,fieldTop+62.f,320.f,46.f};
        cnfPF.rect={cx-160.f,fieldTop+124.f,320.f,46.f};
        saveBtn.rect={cx-130.f,fieldTop+186.f,260.f,44.f};

        DrawTextC(fUIBold,"CHANGE PASSWORD",cx,fieldTop-24.f,11.f,2,CA(C_GOLD,0.60f));
        FieldDraw(oldPF);FieldDraw(newPF);FieldDraw(cnfPF);BtnDraw(saveBtn);
        if(mAlpha>0.01f){float mw=MeasureTextEx(fUI,msg.c_str(),11.f,1).x;DrawTextEx(fUI,msg.c_str(),{cx-mw*0.5f,fieldTop+238.f},11.f,1,CA(mc,mAlpha));}

        // Movie bookings summary
        float ry=fieldTop+260.f;
        if(ry<SH-20.f&&!perMovie.empty()){
            DrawRectangle(28,(int)ry,SW-56,1,CA(C_GOLDDIM,0.22f));ry+=10.f;
            DrawTextEx(fUIBold,"YOUR MOVIE BOOKINGS",{50.f,ry},10.f,2,CA(C_GOLD,0.55f));ry+=20.f;
            for(auto& [midx,cnt]:perMovie){
                if(ry>SH-20.f) break;
                const auto& e=MovieRepo_Get(midx);Color ac=Ac3(e);
                DrawRectangle((int)50.f,(int)ry,(int)(SW-100),28,CA(C_PANEL,0.60f));
                DrawRectangle(50,(int)ry,4,28,CA(ac,0.60f));
                DrawTextEx(fUIBold,TruncText(e.title,fUIBold,10.f,1,400.f).c_str(),{62.f,ry+8.f},10.f,1,C_WHITE);
                char cb2[32];snprintf(cb2,sizeof(cb2),"%d seat%s",cnt,cnt==1?"":"s");
                float cw=MeasureTextEx(fUI,cb2,9.5f,1).x;
                DrawTextEx(fUI,cb2,{(float)(SW-60)-cw,ry+9.f},9.5f,1,CA(C_GOLD,0.75f));
                int ur=RatingRepo_Get(gCurrentUser,e.id);
                if(ur>0){char rs[8];snprintf(rs,sizeof(rs),"%.0f★",(float)ur);
                    DrawTextEx(fUI,rs,{(float)(SW-60)-cw-50.f,ry+9.f},9.5f,1,CA(C_GOLD2,0.80f));}
                ry+=32.f;
            }
        }

        DrawScanlines();
        if(fadeIn<1.f) DrawRectangle(0,0,SW,SH,CA(C_BG,1.f-fadeIn));
        DrawTextC(fUI,"(c) 2025 Movix",SW*0.5f,SH-14.f,9.f,1,CA(C_GREY,0.25f));
        EndDrawing();
    }
}
