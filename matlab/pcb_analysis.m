clear all;
Nrow=47;
Ncol=48;
data=zeros(Nrow,Ncol);
fid=fopen('pcb_data.txt','r');
for ii=1:Nrow
    row=fscanf(fid,'%d',1)
    test=fscanf(fid,'%d',1);
    if (test~=99)
        error('inconsistent format');
    end
    for jj=1:Ncol
        col=fscanf(fid,'%x',1);
        high=fscanf(fid,'%x',1);
        low=fscanf(fid,'%x',1);
        data(row,col)=(high*256+low)/32768*5;
    end
end;
figure;
colormap(1-gray);
imagesc(data,[0 1.8]); colorbar; %range for data display 0V to 1.8V. These numbers can be modified.
save data_matrix.txt data -ascii;
fclose(fid);
figure;
mesh(1:Ncol,1:Nrow,data);
