#include "theme.h"
#include "screens.h"
#include "../bll/session.h"
#include "../bll/movie_service.h"
#include "../bll/booking_service.h"
#include "../dal/movie_repo.h"
#include <cmath>
#include <algorithm>
#include "../dal/rating_repo.h"
#include <cstdio>
#include <vector>

// ── Texture cache ─────────────────────────────────────────────
static std::vector<Texture2D> gTex;
static int gTexFor=-1;
static void RefreshTextures(){
    int cnt=MovieRepo_Count();if(gTexFor==cnt) return;
    for(auto& tx:gTex) if(tx.id) UnloadTexture(tx);
    gTex.assign(cnt,Texture2D{});
    for(int i=0;i<cnt;i++){
        char p[64];snprintf(p,sizeof(p),"posters/movie_%d.jpg",MovieRepo_Get(i).id);
        if(!FileExists(p)) snprintf(p,sizeof(p),"posters/movie_%d.png",MovieRepo_Get(i).id);
        if(FileExists(p)) gTex[i]=LoadTexture(p);
    }
    gTexFor=cnt;
}

static Color Ac(const MovieEntry& e){return {(unsigned char)e.ac_r,(unsigned char)e.ac_g,(unsigned char)e.ac_b,255};}
static Color PA(const MovieEntry& e){return {(unsigned char)e.pA_r,(unsigned char)e.pA_g,(unsigned char)e.pA_b,255};}
static Color PB(const MovieEntry& e){return {(unsigned char)e.pB_r,(unsigned char)e.pB_g,(unsigned char)e.pB_b,255};}

static void DrawPosterProc(float x,float y,float w,float h,const MovieEntry& e,float t){
    Color pa=PA(e),pb=PB(e),ac=Ac(e);
    DrawRectangleGradientV((int)x,(int)y,(int)w,(int)h,pa,pb);
    float pulse=sinf(t*1.4f)*0.5f+0.5f;
    DrawCircle((int)(x+w*0.75f),(int)(y+h*0.28f),(int)(w*0.38f),CA(ac,0.08f+pulse*0.04f));
    DrawCircleLines((int)(x+w*0.75f),(int)(y+h*0.28f),(int)(w*0.38f),CA(ac,0.18f));
    for(int sy=(int)y;sy<(int)(y+h);sy+=4) DrawLine((int)x,sy,(int)(x+w),sy,CA(C_BG,0.10f));
    DrawRectangleGradientV((int)x,(int)(y+h*0.62f),(int)w,(int)(h*0.38f),CA(C_BG,0.f),CA(C_BG,0.88f));
}
static void DrawPoster(float x,float y,float w,float h,int idx,float t){
    const MovieEntry& e=MovieRepo_Get(idx);Color ac=Ac(e);
    if(idx<(int)gTex.size()&&gTex[idx].id){
        Rectangle src={0,0,(float)gTex[idx].width,(float)gTex[idx].height};
        DrawTexturePro(gTex[idx],src,{x,y,w,h},{0,0},0.f,WHITE);
        DrawRectangleGradientV((int)x,(int)(y+h*0.55f),(int)w,(int)(h*0.45f),CA(C_BG,0.f),CA(C_BG,0.90f));
    } else DrawPosterProc(x,y,w,h,e,t);
    char pb[16];snprintf(pb,sizeof(pb),"%.0f BGN",e.price);
    DrawRectangle((int)(x+w-52),(int)(y+8),48,18,CA(C_GOLD,0.92f));
    DrawTextEx(fUIBold,pb,{x+w-48.f,y+11.f},9.5f,1,C_BG);
    DrawRectangle((int)(x+8),(int)(y+8),44,15,CA(ac,0.30f));
    DrawRectangleLinesEx({x+8.f,y+8.f,44.f,15.f},0.8f,CA(ac,0.55f));
    DrawTextEx(fUI,e.year,{x+12.f,y+10.f},9.f,1,CA(C_WHITE,0.85f));
}

static const float CW=195.f,CH=275.f,CGAP=22.f,GCOLS=3.f,GX=30.f,GTOP=90.f;
static const float DX=GX+GCOLS*(CW+CGAP)+20.f, DW=SW-DX-22.f;
struct CA2{float h=0.f,s=0.f;};

// Search bar state
struct SearchBar{Field f;bool active=false;};

void RunMovieScreen(){
    MovieSvc_Init();RefreshTextures();
    float fadeIn=0.f; int sel=0; float sY=0.f,sT=0.f;
    std::vector<CA2> anims(MovieSvc_Count());
    std::vector<int> filtered; // currently displayed indices
    for(int i=0;i<MovieSvc_Count();i++) filtered.push_back(i);

    // Search
    Field searchF; FieldInit(searchF,{GX,GTOP-38.f,GCOLS*(CW+CGAP)-10.f,32.f},"Search movies...",false);
    std::string lastSearch;

    Button bookBtn,logoutBtn,profileBtn,historyBtn,adminBtn;
    BtnInit(bookBtn,   {DX,SH-88.f,DW,48.f},"RESERVE NOW");
    BtnInit(logoutBtn, {(float)(SW-190),20.f,160.f,36.f},"LOGOUT");
    BtnInit(profileBtn,{DX,SH-145.f,DW*0.48f,42.f},"MY PROFILE");
    BtnInit(historyBtn,{DX+DW*0.52f,SH-145.f,DW*0.48f,42.f},"MY BOOKINGS");
    BtnInit(adminBtn,  {(float)(SW-360),20.f,160.f,36.f},"ADMIN PANEL");

    // Rating popup
    bool showRatePopup=false; int rateMovieId=0,rateStars=0; float rateAlpha=0.f;

    while(!WindowShouldClose()){
        float dt=GetFrameTime(),t=GetTime();
        int cnt=MovieSvc_Count();
        if((int)anims.size()!=cnt){anims.resize(cnt);RefreshTextures();}
        if(sel>=cnt) sel=cnt-1; if(sel<0) sel=0;
        fadeIn=std::min(1.f,fadeIn+dt*1.6f);

        // Search update
        FieldUpdate(searchF,dt);
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            searchF.active=CheckCollisionPointRec(GetMousePosition(),searchF.rect);
        if(searchF.buf!=lastSearch){
            lastSearch=searchF.buf;
            filtered=MovieSvc_Search(lastSearch,"");
            if(!filtered.empty()&&std::find(filtered.begin(),filtered.end(),sel)==filtered.end()) sel=filtered[0];
        }

        // Scroll
        float maxScroll=std::max(0.f,std::ceilf((float)filtered.size()/GCOLS)*(CH+CGAP)-(SH-GTOP-20.f));
        sT-=GetMouseWheelMove()*55.f; sT=std::clamp(sT,0.f,maxScroll);
        sY+=(sT-sY)*std::min(1.f,dt*12.f);
        UpdateParticles(dt);

        // Card hover/click
        for(int fi=0;fi<(int)filtered.size();fi++){
            int i=filtered[fi]; int col=fi%(int)GCOLS,row=fi/(int)GCOLS;
            float cx2=GX+(float)col*(CW+CGAP),cy2=GTOP+(float)row*(CH+CGAP)-sY;
            bool hov=cy2>GTOP-CH&&cy2<SH&&CheckCollisionPointRec(GetMousePosition(),{cx2,cy2,CW,CH});
            anims[i].h+=dt*(hov?8.f:-8.f); anims[i].h=std::clamp(anims[i].h,0.f,1.f);
            anims[i].s+=dt*((sel==i)?6.f:-6.f); anims[i].s=std::clamp(anims[i].s,0.f,1.f);
            if(hov&&IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) sel=i;
        }

        // Buttons
        if(BtnUpdate(logoutBtn,dt)){SessionLogout();return;}
        if(BtnUpdate(profileBtn,dt)){RunProfileScreen();MovieSvc_Init();RefreshTextures();}
        if(BtnUpdate(historyBtn,dt)){RunHistoryScreen();}
        if(gIsAdmin&&BtnUpdate(adminBtn,dt)){RunAdminScreen();MovieSvc_Init();anims.resize(MovieSvc_Count());RefreshTextures();}
        if(BtnUpdate(bookBtn,dt)&&cnt>0){
            RunSeatScreen(sel);
            // After booking, prompt rating
            showRatePopup=true; rateMovieId=MovieRepo_Get(sel).id; rateStars=RatingRepo_Get(gCurrentUser,rateMovieId);
            rateAlpha=0.f;
        }

        // Rating popup logic
        if(showRatePopup){
            rateAlpha=std::min(1.f,rateAlpha+dt*3.f);
            int nr=DrawInteractiveStars(0,0,rateStars,0); // dummy, drawn below
            if(IsKeyPressed(KEY_ESCAPE)) showRatePopup=false;
        }

        // ── DRAW ──────────────────────────────────────────────
        BeginDrawing();ClearBackground(C_BG);DrawParticles();
        DrawFilm(0,0,SH,t,false);DrawFilm(SW-28,0,SH,t,true);
        // Header
        DrawRectangle(28,0,SW-56,80,CA(C_PANEL,0.97f));DrawRectangle(28,78,SW-56,2,CA(C_GOLDDIM,0.5f));
        DrawTextC(fTitle,"MOVIX",SW*0.5f,12,38,4,C_GOLD2);
        DrawTextEx(fUI,"SELECT A MOVIE",{48,30},11,3,CA(C_GOLD,0.60f));
        // User info in header
        char ubuf[64];snprintf(ubuf,sizeof(ubuf),"[%s]  %d films",gCurrentUser.c_str(),(int)filtered.size());
        DrawTextEx(fUI,ubuf,{(float)(SW-430),48},9.5f,1,CA(C_GREYLT,0.40f));
        BtnDraw(logoutBtn);
        if(gIsAdmin) BtnDraw(adminBtn);

        // Search bar
        {
            bool sh=CheckCollisionPointRec(GetMousePosition(),searchF.rect);
            DrawRectangleRec(searchF.rect,CA(C_INPUTBG,0.90f));
            DrawRectangleLinesEx(searchF.rect,searchF.active?1.5f:0.8f,searchF.active?CA(C_GOLD,0.70f):CA(C_GOLDDIM,0.35f+sh*0.25f));
            if(searchF.buf.empty()&&!searchF.active)
                DrawTextEx(fUI,"  Search movies...",{searchF.rect.x+8.f,searchF.rect.y+8.f},12.f,1,CA(C_GREY,0.45f));
            else DrawTextEx(fUI,searchF.buf.c_str(),{searchF.rect.x+10.f,searchF.rect.y+8.f},12.f,1,C_WHITE);
        }

        // ── Cards ─────────────────────────────────────────────
        BeginScissorMode((int)GX-4,(int)GTOP,(int)(GCOLS*(CW+CGAP)+8),SH-(int)GTOP);
        for(int fi=0;fi<(int)filtered.size();fi++){
            int i=filtered[fi];int col=fi%(int)GCOLS,row=fi/(int)GCOLS;
            float cx2=GX+(float)col*(CW+CGAP),cy2=GTOP+(float)row*(CH+CGAP)-sY;
            if(cy2+CH<GTOP||cy2>SH) continue;
            const MovieEntry& e=MovieRepo_Get(i); Color ac=Ac(e);
            bool isSel=(sel==i);float hv=anims[i].h,sv=anims[i].s;
            float cardY=cy2-(hv*6.f+sv*4.f);
            DrawRectangle((int)(cx2+4),(int)(cardY+6),(int)CW,(int)CH,CA(C_BG,0.55f));
            DrawPoster(cx2,cardY,CW,CH*0.63f,i,t);
            DrawRectangle((int)cx2,(int)(cardY+CH*0.63f),(int)CW,(int)(CH*0.37f),CA(C_PANEL,0.98f));
            DrawRectangleLinesEx({cx2,cardY,CW,CH},isSel?1.8f:0.9f,isSel?ac:CA(C_GOLDDIM,0.25f+hv*0.40f));
            if(hv>0.05f||sv>0.05f) DrawRectangle((int)cx2,(int)cardY,(int)CW,(int)CH,CA(ac,std::max(hv,sv)*0.07f));
            // User rating indicator (small dot top-left)
            int ur=RatingRepo_Get(gCurrentUser,e.id);
            if(ur>0){char rs[4];snprintf(rs,sizeof(rs),"%d",ur);
                DrawCircle((int)(cx2+CW-16),(int)(cardY+CH*0.63f+8),7,CA(C_GOLD,0.85f));
                DrawTextEx(fUIBold,rs,{cx2+CW-19.f,cardY+CH*0.63f+3.f},8.f,1,C_BG);}
            float bx=cx2+10.f,by=cardY+CH*0.63f+7.f,maxTW=CW-20.f;
            DrawTextEx(fUIBold,TruncText(e.title,fUIBold,10.5f,1,maxTW).c_str(),{bx,by},10.5f,1,C_WHITE);
            DrawTextEx(fUI,TruncText(e.genre,fUI,9.f,1,maxTW).c_str(),{bx,by+14.f},9.f,1,CA(C_GREYLT,0.65f));
            DrawStars(bx,by+29.f,e.rating,7.5f,ac);
            char rs2[8];snprintf(rs2,sizeof(rs2),"%.1f",e.rating);
            DrawTextEx(fUI,rs2,{bx+46.f,by+30.f},8.f,1,CA(C_GOLD,0.70f));
            DrawTextEx(fUI,e.duration,{bx,by+45.f},8.5f,1,CA(C_GREY,0.48f));
        }
        EndScissorMode();

        // ── Detail panel ──────────────────────────────────────
        if(cnt>0){
            const MovieEntry& e=MovieRepo_Get(sel); Color ac=Ac(e);
            float dx=DX,dy=GTOP+8.f,dw=DW;
            DrawPoster(dx,dy,dw,190.f,sel,t); dy+=190.f+14.f;
            BeginScissorMode((int)dx,(int)dy,(int)dw,(int)(SH-dy-200.f));
            for(auto& tl:WrapText(e.title,fUIBold,14.5f,1.5f,dw)){DrawTextEx(fUIBold,tl.c_str(),{dx,dy},14.5f,1.5f,C_GOLD2);dy+=19.f;}
            DrawTextEx(fUI,e.genre,{dx,dy},10.5f,1,CA(C_GREYLT,0.75f));dy+=16.f;
            // Community avg + user rating
            float avg=RatingRepo_Avg(e.id);
            DrawStars(dx,dy,e.rating,9.f,ac);
            char rf[40];snprintf(rf,sizeof(rf),"%.1f/10",e.rating);
            DrawTextEx(fUIBold,rf,{dx+54.f,dy},9.5f,1,CA(C_GOLD,0.80f));
            if(avg>0){char av[32];snprintf(av,sizeof(av),"Community: %.1f",avg);
                DrawTextEx(fUI,av,{dx+110.f,dy+1.f},8.5f,1,CA(C_GREYLT,0.55f));}dy+=17.f;
            DrawRectangle((int)dx,(int)dy,(int)dw,1,CA(C_GOLDDIM,0.25f));dy+=7.f;
            auto MR=[&](const char* lbl,const char* val){
                DrawTextEx(fUIBold,lbl,{dx,dy},9.f,2,CA(C_GOLD,0.55f));
                DrawTextEx(fUI,TruncText(val,fUI,10.f,1,dw-80.f).c_str(),{dx+80.f,dy},10.f,1,C_WHITE);dy+=15.f;};
            MR("DIRECTOR",e.director);MR("DURATION",e.duration);MR("YEAR",e.year);dy+=2.f;
            DrawRectangle((int)dx,(int)dy,(int)dw,1,CA(C_GOLDDIM,0.22f));dy+=7.f;
            DrawTextEx(fUIBold,"CAST",{dx,dy},9.f,2,CA(C_GOLD,0.55f));dy+=12.f;
            for(auto& cl:WrapText(e.cast,fUI,10.f,1,dw)){DrawTextEx(fUI,cl.c_str(),{dx+6.f,dy},10.f,1,CA(C_GREYLT,0.80f));dy+=12.f;}dy+=4.f;
            DrawTextEx(fUIBold,"SYNOPSIS",{dx,dy},9.f,2,CA(C_GOLD,0.55f));dy+=12.f;
            for(auto& dl:WrapText(e.desc,fUI,10.f,1,dw)){DrawTextEx(fUI,dl.c_str(),{dx,dy},10.f,1,CA(C_GREYLT,0.70f));dy+=12.f;}
            // User's bookings for this film
            auto myRes=ResRepo_ForUser(gCurrentUser);
            std::string mySeats;
            for(auto& r:myRes) if(r.movieIdx==sel){char s[8];snprintf(s,sizeof(s),"%c%d",'A'+r.row,r.col+1);mySeats+=(mySeats.empty()?"":"  ")+std::string(s);}
            if(!mySeats.empty()){dy+=5.f;DrawTextEx(fUIBold,"YOUR SEATS",{dx,dy},8.5f,2,CA(ac,0.65f));dy+=12.f;DrawTextEx(fUI,mySeats.c_str(),{dx+4.f,dy},10.f,1,CA(ac,0.85f));}
            EndScissorMode();

            // Price bar
            float pBy=SH-200.f;
            char pf[40];snprintf(pf,sizeof(pf),"TICKET PRICE:  %.2f BGN",e.price);
            DrawRectangle((int)dx,(int)pBy,(int)dw,30,CA(C_GOLD,0.12f));DrawRectangleLinesEx({dx,pBy,dw,30.f},1.f,CA(C_GOLD,0.35f));
            float pw2=MeasureTextEx(fUIBold,pf,12.f,2).x;DrawTextEx(fUIBold,pf,{dx+(dw-pw2)*0.5f,pBy+8.f},12.f,2,C_GOLD2);

            // User rating below price
            float rY=SH-162.f;
            int ur2=RatingRepo_Get(gCurrentUser,e.id);
            DrawTextEx(fUIBold,"YOUR RATING:",{dx,rY},9.f,2,CA(C_GOLD,0.55f));
            int nr2=DrawInteractiveStars(dx+110.f,rY,ur2,14.f);
            if(nr2!=ur2) MovieSvc_Rate(gCurrentUser,e.id,nr2);
            if(ur2==0) DrawTextEx(fUI,"(click to rate)",{dx+110.f+5*18.f+8.f,rY+2.f},9.f,1,CA(C_GREY,0.40f));

            BtnDraw(profileBtn);BtnDraw(historyBtn);
            bookBtn.rect={dx,SH-88.f,dw,48.f};BtnDraw(bookBtn);
        }

        // ── Rate popup (shown after booking) ──────────────────
        if(showRatePopup&&rateAlpha>0.01f){
            float a=rateAlpha;
            float pw3=400.f,ph3=160.f,px3=(SW-pw3)*0.5f,py3=(SH-ph3)*0.45f;
            DrawRectangle(0,0,SW,SH,CA(C_BG,a*0.75f));
            DrawRectangleRounded({px3,py3,pw3,ph3},0.08f,8,CA(C_PANEL,0.98f));
            DrawRectangleLinesEx({px3,py3,pw3,ph3},2.f,CA(C_GOLD,a));
            DrawTextC(fUIBold,"Rate this movie!",px3+pw3*0.5f,py3+18.f,14.f,2,CA(C_GOLD2,a));
            DrawTextC(fUI,"Your rating helps other viewers",px3+pw3*0.5f,py3+42.f,10.f,1,CA(C_GREYLT,a*0.65f));
            float starX=px3+(pw3-5*22.f)*0.5f;
            int nr3=DrawInteractiveStars(starX,py3+68.f,rateStars,18.f);
            if(nr3!=rateStars){rateStars=nr3;MovieSvc_Rate(gCurrentUser,rateMovieId,rateStars);}
            DrawTextC(fUI,"Press ESC to close",px3+pw3*0.5f,py3+ph3-22.f,9.f,1,CA(C_GREY,a*0.45f));
            if(IsKeyPressed(KEY_ESCAPE)||IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                Vector2 m=GetMousePosition();
                if(!CheckCollisionPointRec(m,{px3,py3,pw3,ph3})) showRatePopup=false;
            }
        }

        DrawScanlines();
        if(fadeIn<1.f) DrawRectangle(0,0,SW,SH,CA(C_BG,1.f-fadeIn));
        DrawTextC(fUI,"(c) 2025 Movix",SW*0.5f,SH-14.f,9.f,1,CA(C_GREY,0.25f));
        EndDrawing();
    }
    for(auto& tx:gTex) if(tx.id) UnloadTexture(tx);
    gTex.clear();gTexFor=-1;
}
