#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include<iostream>

using namespace cv;
using namespace std;


Mat imgOrginal,imgGray,imgCanny,imgBlur,imgDia,imgErode,imgThre,imgWarp, imgCrop;
//VideoCapture cap(0);
vector<Point> initialPoints,docsPoints;
float w=420,h=596;

Mat preProcessing(Mat img)
{
    Mat kernel=getStructuringElement(MORPH_RECT,Size(5,5));
    cvtColor(img, imgGray, COLOR_BGR2GRAY);
    GaussianBlur(img,imgBlur,Size(7,7),5,0);
    Canny(imgBlur,imgCanny,25,75);
    dilate(imgCanny,imgDia,kernel);
    //erode(imgDia,imgErode,kernel);
    return imgDia;
}

vector<Point> getcontours(Mat imgDia)
{
    /*{{Point(20,30),Point(50,60)},
    {},
    {}}*/
    vector<vector<Point>> contours;
    vector<Vec4i>hierarchy;
    findContours(imgDia,contours,hierarchy,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
    //drawContours(img,contours,-1,Scalar(255,0,255),2);
    vector<vector<Point>> conPoly(contours.size());
    vector<Rect> boundRect(contours.size());
    vector<Point> biggest;
    //variations
    int maxArea=0;
    for (int i = 0; i < contours.size(); i++)
    {
        int area=contourArea(contours[i]);
        cout<<area<<endl;
        
        
        string objType;

        if(area>2000)
        {
            float peri=arcLength(contours[i],true);
            approxPolyDP(contours[i],conPoly[i],0.02*peri,true);
            if (area>maxArea && conPoly[i].size()==4)
            {
                //drawContours(imgOrginal,conPoly,i,Scalar(255,0,255),5);
                biggest={conPoly[i][0],conPoly[i][1],conPoly[i][2],conPoly[i][3]};
                maxArea=area;
            }
            
            /*cout<<conPoly[i].size()<<endl;
            boundRect[i]=boundingRect(conPoly[i]);
            myPoint.x=boundRect[i].x+boundRect[i].width/2;
            myPoint.y=boundRect[i].y;*/
           /* int objCor=(int) conPoly[i].size();

            if(objCor==3){objType="Triangle";}
            else if (objCor==4){
                float aspRatio= (float)boundRect[i].width/(float)boundRect[i].height;
                cout<<aspRatio<<endl;
                if(aspRatio>0.95 && aspRatio<1.05){objType="Sqare";}
                else {objType="Rectangle";}
                }
            else if (objCor>4) {objType="Circle";}
            */
            //drawContours(imgOrginal,conPoly,i,Scalar(255,0,255),2);
            //rectangle(img,boundRect[i].tl(),boundRect[i].br(),Scalar(0,255,0),5);
            //putText(img,objType,{boundRect[i].x,boundRect[i].y-5},FONT_HERSHEY_DUPLEX,0.25,Scalar(0,69,255),2);
        }
    }
    return biggest;
}

void drawPoints(vector<Point> points,Scalar color)
{
    for(int i=0;i<points.size();i++)
    {
        circle(imgOrginal,points[i],10,color,FILLED);
        putText(imgOrginal,to_string(i),points[i],FONT_HERSHEY_PLAIN,4,color,4);
    }
}

vector<Point> reorder(vector<Point> points)
{
    vector<Point> newPoints;
    vector<int> sumPoints,subPoints;
    for (int i = 0; i < 4; i++)
    {
        sumPoints.push_back(points[i].x+points[i].y);
        subPoints.push_back(points[i].x-points[i].y);
    }
    newPoints.push_back(points[min_element(sumPoints.begin(),sumPoints.end())-sumPoints.begin()]);//0
    newPoints.push_back(points[max_element(subPoints.begin(),subPoints.end())-subPoints.begin()]);//1
    newPoints.push_back(points[min_element(subPoints.begin(),subPoints.end())-subPoints.begin()]);//2
    newPoints.push_back(points[max_element(sumPoints.begin(),sumPoints.end())-sumPoints.begin()]);//3

    return newPoints;
}

Mat getWarp(Mat img, vector<Point> points,float w,float h)
{
    Point2f src[4]={points[0],points[1],points[2],points[3]};
    Point2f dst[4]={{0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h}};

    Mat matrix=getPerspectiveTransform(src,dst);
    warpPerspective(img,imgWarp,matrix,Point(w,h));
    return imgWarp;
}

int main()
{
    string path="paper.jpg";
    imgOrginal=imread(path);
    //cap.read(imgOrginal);
    //resize(imgOrginal,imgOrginal,Size(),0.5,0.5);
    //preprocessing
    imgThre=preProcessing(imgOrginal);
    imshow("Image Dilation",imgThre);
    //Get Contours - Biggest
    initialPoints=getcontours(imgThre);
    //drawPoints(initialPoints,Scalar(0,0,255));
    docsPoints = reorder(initialPoints);
    //drawPoints(docsPoints,Scalar(0,255,0));
    //Warp 
    imgWarp=getWarp(imgOrginal,docsPoints,w,h);
    //crop
    int cropVal=5;
    Rect roi(cropVal,cropVal,w-(2*cropVal),h-(2*cropVal));
    imgCrop=imgWarp(roi);
    imshow("Image",imgOrginal);
    imshow("Image Wrap",imgWarp);
    imshow("Image Crop",imgCrop);
    waitKey(0);
    
    return 0;
}