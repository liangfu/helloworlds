function gdsolver()
% GDSOLVER demostrates gradient descent algorithm on Wikipedia

fx0=inline('3*x(1)-cos(x(2)*x(3))-1.5');
fx1=inline('4*x(1)*x(1)-625*x(2)*x(2)+2*x(2)-1');
fx2=inline('exp(-x(1)*x(2))+20*x(3)+(10*pi-3)/3.');
Gx=inline('[fx0(x);fx1(x);fx2(x)]','x','fx0','fx1','fx2');
% eval by calling `Gx(x0,fx0,fx1,fx2)`
Fx=inline('.5*Gx(x,fx0,fx1,fx2).*Gx(x,fx0,fx1,fx2)',...
          'x','Gx','fx0','fx1','fx2');
% eval by calling `Fx(x0,Gx,fx0,fx1,fx2)`
dfx0=inline('[3,sin(x(2)*x(3))*x(3),sin(x(2)*x(3))*x(2)]');
dfx1=inline('[8*x(1),-1250*x(2)+2,0]');
dfx2=inline('[-x(2)*exp(-x(1)*x(2)),-x(1)*exp(-x(1)*x(2)),20]');
J_G=inline('[dfx0(x);dfx1(x);dfx2(x)]','x','dfx0','dfx1','dfx2');
% eval by calling `J_G(x0,dfx0,dfx1,dfx2)`
dFx=inline('(J_G(x,dfx0,dfx1,dfx2))''*Gx(x,fx0,fx1,fx2)',...
           'x','J_G','dfx0','dfx1','dfx2','Gx','fx0','fx1','fx2');
% eval by calling `dFx(x0,J_G,dfx0,dfx1,dfx2,Gx,fx0,fx1,fx2)`

% parameters
gamma=1e-3;
x0=[0;0;0];
epsilon=0.1;
maxiter=1000;

% initialize
iter=2;
xi=x0;
yi=sum(Fx(xi,Gx,fx0,fx1,fx2)); % eval objective

% start iteration
while iter<maxiter && yi(end)>epsilon
xi=xi-gamma*dFx(xi,J_G,dfx0,dfx1,dfx2,Gx,fx0,fx1,fx2);
yi(iter)=sum(Fx(xi,Gx,fx0,fx1,fx2)); % eval objective
iter=iter+1;
end
plot(yi);

end

