clc;
clear all;
close all;
source = VideoReader('test.mp4');
writerobj=VideoWriter('result.avi');
open(writerobj);
fr = read(source,1);
fr_size = size(fr);
height = fr_size(1);
width = fr_size(2);

// 设置帧数
fra_num = 690;
startframe=1;
fra_num_training =255;
fra_num_testing = 435;

CTU_size = 64;
if(mod(height,CTU_size)==0)
    H = height/CTU_size;
else
    H = floor(height/CTU_size)+1;
end

if(mod(width,CTU_size)==0)
    W = width/CTU_size;
else
    W = floor(width/CTU_size)+1;
end

//分别是奇数帧、偶数被二整除帧、偶数被四整除帧
data1_index=[];
data2_index=[];
data3_index=[];
% for k=1:fra_num-startframe+1
for k=startframe:fra_num
    if mod(k,2)~=0
        data1_index=[data1_index,k];
    else
        if mod(k,2)==0 && mod(k,4)~=0
            data2_index=[data2_index,k];
        else
            data3_index=[data3_index,k];
        end
    end
end


fid1 = fopen('data1.txt');
fid2=fopen('data2.txt');
fid3=fopen('data3.txt');

for k = 1:length(data1_index)
    for i = 1:H
        for j = 1:W
            data1(i,j,k) = fscanf(fid1,'%d',1);
        end
    end
end
for k = 1:length(data2_index)
    for i = 1:H
        for j = 1:W
            data2(i,j,k) = fscanf(fid2,'%d',1);
        end
    end
end
for k = 1:length(data3_index)
    for i = 1:H
        for j = 1:W
            data3(i,j,k) = fscanf(fid3,'%d',1);
        end
    end
end


% for k = 1:fra_num_training
%      for i = 1:H
%         for j = 1:W
%             data1(i,j,k) = data(i,j,k);
%         end
%     end
% end
%
% for k = 1:fra_num_testing
%      for i = 1:H
%         for j = 1:W
%             data2(i,j,k) = data(i,j,k+fra_num_training);
%         end
%     end
% end
%��˹ģ�Ͳ���
% max_k = 10;
% max_k =5;
max_k =5;
w = zeros(H,W,max_k);
m = zeros(H,W,max_k);
sd = zeros(H,W,max_k);
comp_num = zeros(H,W);




operateoutlier=false;
guassianfilter=true;
%use neighbourhood average bits
windowsize=3;
 gt=zeros(H,W,fra_num);
 hasgt=true;
 if(hasgt)
   for frameindex=2:fra_num
        groundtruthfile=sprintf('%s%s%d%s%s','F:\project\gem\gem2\groundtruth','/gt00',1103+frameindex,'.png');
        gts=imread(groundtruthfile);
        gt(:,:,frameindex)=gts(:,:);
   end
 end




%test neighbour operate noise

%use wavelet
usewavelet=false;
if usewavelet
for ver=1:H
    for hor=1:W
    sig = data_after(ver,hor,2:fra_num);

    % ?????
    [c,l]=wavedec(sig,4,'db4');

    %?????????????
    a1=appcoef(c,l,'db4',1);
    d1=detcoef(c,l,1);
    a2=appcoef(c,l,'db4',2);
    d2=detcoef(c,l,2);
    a3=appcoef(c,l,'db4',3);
    d3=detcoef(c,l,3);
    a4=appcoef(c,l,'db4',4);
    d4=detcoef(c,l,4);

    % ????????????????????????
    dd1=zeros(size(d1));
    dd2=zeros(size(d2));
    c1=[a4 d4 d3 dd2 dd1];
    aa1=waverec(c1,l,'db4');
    data_after(ver,hor,2:fra_num)=max(0,aa1);
    end
end
end

% for i=1:H
%         for j=1:W
%              X=shiftdim(data_after(i,j,2:fra_num));
%              gmm_input=X;
%              [IDX, isnoise]=DBSCAN(gmm_input,6,3);
%              isnoise_after(i,j,:)=[1,isnoise'];
%              actual = gmm_input(find(isnoise == 0));
%              [w_,m_,sd_] = gmmbvl_em(actual,max_k,max_k,1,1,0);
%              comp_num_after(i,j) = size(w_,1);
%
%             for k = 1:comp_num_after(i,j)
%                 w_after(i,j,k) = w_(k);
%                 m_after(i,j,k) = m_(k);
%                 sd_after(i,j,k) = sd_(k);
%             end
%
%         %after operate noise apply guassian filter
%         X=shiftdim(data(i,j,2:fra_num))
%         X_filtered=Gaussianfilter(3,0.5,X');
%         data_filtered(i,j,2:fra_num)=X_filtered;
%          gmm_input=X_filtered';
%             [IDX, isnoise]=DBSCAN(gmm_input,6,3);
%              isnoise_all_filtered(i,j,:)=[1,isnoise'];
%              actual = gmm_input(find(isnoise == 0));
%              [w_,m_,sd_] = gmmbvl_em(actual,max_k,max_k,1,1,0);
%              comp_num_filtered(i,j) = size(w_,1);
%
%             for k = 1:comp_num_filtered(i,j)
%                 w_filtered(i,j,k) = w_(k);
%                 m_filtered(i,j,k) = m_(k);
%                 sd_filtered(i,j,k) = sd_(k);
%             end
%         end
% end
 %test end

   [w1,m1,sd1,comp_num1,isnoise1]=getdistribution(H,W,data1,gt(:,:,data1_index),max_k)
   [w2,m2,sd2,comp_num2,isnoise2]=getdistribution(H,W,data2,gt(:,:,data2_index),max_k)
   [w3,m3,sd3,comp_num3,isnoise3]=getdistribution(H,W,data3,gt(:,:,data3_index),max_k)

for i = 1:H
    for j = 1:W
       rank1 = zeros(1,max_k);
       rank2 = zeros(1,max_k);
       rank3 = zeros(1,max_k);

       for k = 1:comp_num1(i,j)
                rank1(k) = w1(i,j,k)/sd1(i,j,k);
       end
       for k = 1:comp_num2(i,j)
                rank2(k) = w2(i,j,k)/sd2(i,j,k);
       end
       for k = 1:comp_num3(i,j)
                rank3(k) = w3(i,j,k)/sd3(i,j,k);
       end
%        for ii = 1:comp_num(i,j)
%             for jj = 1:comp_num(i,j)-ii
%                 if(rank(i,j,jj) < rank(i,j,jj+1))
%                     temp = w(i,j,jj);
%                     w(i,j,jj) = w(i,j,jj+1);
%                     w(i,j,jj+1) = temp;
%
%                     temp = m(i,j,jj);
%                     m(i,j,jj) = m(i,j,jj+1);
%                     m(i,j,jj+1) = temp;
%
%                     temp = sd(i,j,jj);
%                     sd(i,j,jj) = sd(i,j,jj+1);
%                     sd(i,j,jj+1) = temp;
%                 end
%             end
%        end
       [b,index]=sort(rank1,'descend');
       w1(i,j,:)=w1(i,j,index);
       sd1(i,j,:)=sd1(i,j,index);
       m1(i,j,:)=m1(i,j,index);
       [b,index]=sort(rank2,'descend');
       w2(i,j,:)=w2(i,j,index);
       sd2(i,j,:)=sd2(i,j,index);
       m2(i,j,:)=m2(i,j,index);
       [b,index]=sort(rank3,'descend');
       w3(i,j,:)=w3(i,j,index);
       sd3(i,j,:)=sd3(i,j,index);
       m3(i,j,:)=m3(i,j,index);
%         [b,index]=sort(rank_filtered,'descend');
%        w_filtered(i,j,:)=w_filtered(i,j,index);
%        sd_filtered(i,j,:)=sd_filtered(i,j,index);
%        m_filtered(i,j,:)=m_filtered(i,j,index);
%         [b,index]=sort(rank_after,'descend');
%        w_after(i,j,:)=w_after(i,j,index);
%        sd_after(i,j,:)=sd_after(i,j,index);
%        m_after(i,j,:)=m_after(i,j,index);

    end
end
main_w_thres=1;
drawplot=true;
thres1=-1;
thres2=-1;
thres3=-1;
if drawplot
%plot guassian distribution

    ver=4;
    hor=2;
%     ver=4;
%     hor=5;
%     ver=3;
%     hor=5;

%     plotfrequency(groundtruthfilepath,ver,hor,data,data1,data2,m,sd,m1,sd1,m2,sd2,max_k,fra_num,xmin,xmax,ymin,ymax,index1,index2,index)
%     vec=data(ver,hor,:);
%      gt=zeros(1,fra_num);
%    for frameindex=2:fra_num
%         groundtruthfile=sprintf('%s%s%d%s%s','F:\project\gem\gem2\groundtruth','/gt00',1103+frameindex,'.png');
%         gts=imread(groundtruthfile);
%         gt(frameindex)=gts(ver,hor);
%    end
%     plotforonemodel(vec,gt,2,1:size(vec,3),max_k,m,sd,ver,hor);

%  vec=data(ver,hor,:);
     gt=zeros(1,fra_num);
   for frameindex=2:fra_num
        groundtruthfile=sprintf('%s%s%d%s%s','F:\project\gem\gem2\groundtruth','/gt00',1103+frameindex,'.png');
        gts=imread(groundtruthfile);
        gt(frameindex)=gts(ver,hor);
   end
%     plotforonemodel(data1(ver,hor,:),gt,2,data1_index,max_k,m1,sd1,w1,ver,hor,main_w_thres);
%     plotforonemodel(data2(ver,hor,:),gt,3,data2_index,max_k,m2,sd2,w2,ver,hor,main_w_thres);
%     plotforonemodel(data3(ver,hor,:),gt,4,data3_index,max_k,m3,sd3,w3,ver,hor,main_w_thres);
    plotforonemodel(data1(ver,hor,find(isnoise1(ver,hor,:)==0)),gt,2,data1_index(find(isnoise1(ver,hor,:)==0)),max_k,m1,sd1,w1,ver,hor,main_w_thres);
    plotforonemodel(data2(ver,hor,find(isnoise2(ver,hor,:)==0)),gt,3,data2_index(find(isnoise2(ver,hor,:)==0)),max_k,m2,sd2,w2,ver,hor,main_w_thres);
    plotforonemodel(data3(ver,hor,find(isnoise3(ver,hor,:)==0)),gt,4,data3_index(find(isnoise3(ver,hor,:)==0)),max_k,m3,sd3,w3,ver,hor,main_w_thres);
end


groupinfo=group_use_emd(m1,sd1,w1,comp_num1);




%  fr = read(source,1);
%���
data=zeros(H,W,fra_num);
data(:,:,data1_index)=data1(:,:,:);
data(:,:,data2_index)=data2(:,:,:);
data(:,:,data3_index)=data3(:,:,:);
thres=-1;
threshold1=zeros(H,W);
threshold2=zeros(H,W);
threshold3=zeros(H,W);
% main_w_thres=1;
for n = 1:fra_num
    fr = read(source,n);          % read in frame
    fr_bw = rgb2gray(fr);       % convert frame to grayscale
    tag = zeros(H,W);
    if(ismember(n,data1_index))
        comp_num=comp_num1;
        w=w1;
        sd=sd1;
        m=m1;
%         thres=thres1;
    end
    if(ismember(n,data2_index))
        comp_num=comp_num2;
        w=w2;
        sd=sd2;
        m=m2;
%          thres=thres2;
    end
    if(ismember(n,data3_index))
        comp_num=comp_num3;
         w=w3;
        sd=sd3;
        m=m3;
%           thres=thres3;
    end
    for i = 1:H
        for j = 1:W
            total_w = 0;
            %%%original operation
            for k_max = 1:comp_num(i,j)
                total_w = total_w + w(i,j,k_max);
                if(total_w > main_w_thres)
                    break;
                end
            end
            thresh1 = -1;
            for k = 1:k_max
                if(thresh1 < m(i,j,k) + 3*sd(i,j,k))
                   thresh1 = m(i,j,k) +3*sd(i,j,k);
                end
            end
            if(data(i,j,n) > thresh1)
                     tag(i,j)=99;
            end
            if(ismember(n,data1_index))
                threshold1(i,j)=thresh1;
            end
            if(ismember(n,data2_index))
                threshold2(i,j)=thresh1;
            end
            if(ismember(n,data3_index))
                threshold3(i,j)=thresh1;
            end
%             if(data(i,j,n) > thresh)
%                      tag(i,j)=99;
%             end
        end
    end



    for i = 1:H
        for j = 1:W
            if(tag(i,j) == 99)
                for a = 1:64
                     for b = 1:64
                             fr((i-1)*64+a, (j-1)*64+b,1) = fr((i-1)*64+a, (j-1)*64+b,1)*0.5+255*0.5;  %�ж�Ϊǰ����������64*64����Ϊ��ɫ
%                              fg((i-1)*64+a, (j-1)*64+b,2)=0;
%                              fg((i-1)*64+a, (j-1)*64+b,3)=0;
                     end
                end
            end
        end
    end

%     for j = 1:W
%         if(tag(H,j)==99)
%              for a = 1:48
%                   for b = 1:64
%                        fg(3*64+a,(j-1)*64+b) = 255;  %�ж�Ϊǰ����������64*64����Ϊ��ɫ
%                   end
%              end
%         else
%              for a = 1:48
%                   for b = 1:64
%                        fg(3*64+a,(j-1)*64+b) = fr_bw(3*64+a,(j-1)*64+b);  %�ж�Ϊ������������64*64����Ϊ��ɫ
%                   end
%              end
%         end
%     end

    figure(5);
    writeVideo(writerobj,uint8(fr));
    imshow(uint8(fr)) ;
end
close(writerobj);
threshold1
threshold2
threshold3

function [w,m,sd,comp_num,noise]=getdistribution(H,W,data,gt,max_k)
noise=zeros(H,W,size(data,3));
for i=1:H
        for j=1:W
%              X=shiftdim(data(i,j,2:fra_num));
             bg=find(gt(i,j,:)==0);
             X=shiftdim(data(i,j,bg));

             gmm_input=X;
% % % % % % % % % % % % % % % % % % % % %
%              [IDX, isnoise]=DBSCAN(gmm_input,6,3);
%              noise(i,j,bg)=isnoise;
%              actual = gmm_input(find(isnoise == 0));
% % % % % % % % % % % % % % % % % % % % %
               actual=gmm_input;
% % % % % % % % % % % % % % % % % % % % %
%              [w_,m_,sd_] = gmmbvl_em(actual,max_k,max_k,1,1,0);
             [w_,m_,sd_] = gmmbvl_em(actual,max_k,max_k,0,1,0);
             comp_num(i,j) = size(w_,1);

            for k = 1:comp_num(i,j)
                w(i,j,k) = w_(k);
                m(i,j,k) = m_(k);
                sd(i,j,k) = sd_(k);
            end
        end
  end
end

