N[$.nlp!=""]{
 double x, y, lx, ly;
 sscanf($.pos,"%f,%f",&x,&y);
 sscanf($.nlp,"%f,%f",&lx,&ly);
 $.xlp = sprintf("%.03f,%.03f",x+lx,y+ly);
}
