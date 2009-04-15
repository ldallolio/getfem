gf_workspace('clear all');
disp('2D scalar wave equation (helmholtz) demonstration');
disp(' we present two approaches for the solution of the helmholtz problem')
disp(' - the first one is to use the getfem "model bricks"')
disp(' - the second is to use the "low level" approach, i.e. to assemble')
disp(' and solve the linear systems.')

disp('The result is the wave scattered by a disc, the incoming wave beeing a plane wave coming from the top');
disp(' \delta u + k^2 = 0');
disp(' u = -uinc              on the interior boundary');
disp(' \partial_n u + iku = 0 on the exterior boundary');

%PK = 10; gt_order = 6; k = 7; use_hierarchical = 0; load_the_mesh=0;
PK=3; gt_order = 3; k = 1; use_hierarchical = 1; load_the_mesh=1;

if (use_hierarchical) s = 'hierarchical'; else s = 'classical'; end;
disp(sprintf('using %s P%d FEM with geometric transformations of degree %d',s,PK,gt_order));
if (load_the_mesh),
  disp('the mesh is loaded from a file, gt_order ignored');
end;
if load_the_mesh == 0,
  % a quadrangular mesh is generated, with a high degree geometric transformation
  % number of cells for the regular mesh
  Nt=10; Nr=8;
  m=gfMesh('empty',2);
  dtheta=2*pi*1/Nt; R=1+9*(0:Nr-1)/(Nr-1);
  gt=gfGeoTrans(sprintf('GT_PRODUCT(GT_PK(1,%d),GT_PK(1,1))',gt_order));
  ddtheta=dtheta/gt_order;
  for i=1:Nt;
    for j=1:Nr-1;
      ti=(i-1)*dtheta:ddtheta:i*dtheta;
      X = [R(j)*cos(ti) R(j+1)*cos(ti)];
      Y = [R(j)*sin(ti) R(j+1)*sin(ti)];
      m.set('add convex',gt,[X;Y]);
    end;
  end;
  fem_u=gfFem(sprintf('FEM_QK(2,%d)',PK));
  fem_d=gfFem(sprintf('FEM_QK(2,%d)',PK));
  mfu=gfMeshFem(m,1);
  mfd=gfMeshFem(m,1);  
  mfu.set('fem',fem_u);
  mfd.set('fem',fem_d);
  sIM=sprintf('IM_GAUSS_PARALLELEPIPED(2,%d)',gt_order+2*PK);
  mim=gfMeshIm(m, gfInteg(sIM));
else
  % the mesh is loaded
  m=gfMesh('import','gid','../meshes/holed_disc_with_quadratic_2D_triangles.msh');
  if (use_hierarchical),
    % hierarchical basis improve the condition number
    % of the final linear system
    fem_u=gfFem(sprintf('FEM_PK_HIERARCHICAL(2,%d)',PK));
    %fem_u=gfFem('FEM_HCT_TRIANGLE');
    %fem_u=gfFem('FEM_HERMITE(2)');
  else,
    fem_u=gfFem(sprintf('FEM_PK(2,%d)',PK));
  end;
  fem_d=gfFem(sprintf('FEM_PK(2,%d)',PK));
  mfu=gfMeshFem(m,1);
  mfd=gfMeshFem(m,1);  
  set(mfu,'fem',fem_u);
  set(mfd,'fem',fem_d);
  mim=gfMeshIm(m,gfInteg('IM_TRIANGLE(13)'));
end;
nbdu=mfu.nbdof;
nbdd=mfd.nbdof;

% identify the inner and outer boundaries
P=m.pts; % get list of mesh points coordinates
pidobj=find(sum(P.^2) < 1*1+1e-6);
pidout=find(sum(P.^2) > 10*10-1e-2);
% build the list of faces from the list of points
fobj=get(m,'faces from pid',pidobj); 
fout=get(m,'faces from pid',pidout);
set(m,'boundary',1,fobj);
set(m,'boundary',2,fout);

% expression of the incoming wave
wave_expr=sprintf('cos(%f*y+.2)+1i*sin(%f*y+.2)',k,k);
Uinc=get(mfd,'eval',{wave_expr});


%
% we present two approaches for the solution of the helmholtz problem
% - the first one is to use the getfem "model bricks"
% - the second is to use the "low level" approach, i.e. to assemble
% and solve the linear systems.
if 1,
  % solution using model bricks
  b0=gfMdBrick('helmholtz',mim,mfu);
  set(b0,'param','wave_number', k);
  b1=gfMdBrick('dirichlet',b0, 1, mfd, 'augmented');
  set(b1,'param','R',mfd,Uinc);
  b2=gfMdBrick('qu term',b1, 2); set(b2, 'param','Q',1i*k);
  
  mds=gfMdState(b2);
  
  t0=cputime; 
  get(b2, 'solve', mds, 'noisy');
  disp(sprintf('solve done in %.2f sec', cputime-t0));
  U=get(mds, 'state'); U=U(1:mfu.nbdof);
else
  % solution using the "low level" approach
  [H,R] = gf_asm('dirichlet', 1, mim, mfu, mfd, gf_mesh_fem_get(mfd,'eval',1),Uinc);
  [null,ud]=gf_spmat_get(H,'dirichlet nullspace', R);
  
  Qb2 = gf_asm('boundary qu term', 2, mim, mfu, mfd, ones(1,nbdd));
  M = gf_asm('mass matrix',mim, mfu);
  L = -gf_asm('laplacian',mim, mfu,mfd,ones(1,nbdd));

  % builds the matrix associated to
  % (\Delta u + k^2 u) inside the domain, and 
  % (\partial_n u + ik u) on the exterior boundary
  A=L + (k*k) * M + (1i*k)*Qb2;


  % eliminate dirichlet conditions and solve the system
  RF=null'*(-A*ud(:));
  RK=null'*A*null;
  U=null*(RK\RF)+ud(:);
  U=U(:).';
end;

Ud=gf_compute(mfu,U,'interpolate on',mfd);

%figure(1); gf_plot(mfu,imag(U(:)'),'mesh','on','refine',32,'contour',0); colorbar;
%figure(2); gf_plot(mfd,abs(Ud(:)'),'mesh','on','refine',24,'contour',0.5); colorbar;


% compute the "exact" solution from its developpement 
% of bessel functions:
% by \Sum_n c_n H^(1)_n(kr)exp(i n \theta)
N=1000; theta=2*pi*(0:N-1)/N; y=sin(theta); 
w = eval(wave_expr);
fw = fft(w); C=fw/N;
S = zeros(size(w)); S(:) = C(1); Nc=20;
for i=2:Nc, 
  n=i-1;  
  S = S + C(i)*exp(1i*n*theta) + C(N-(n-1))*exp(-1i*n*theta);
end;
P=gf_mesh_fem_get(mfd,'basic dof nodes');
[T,R]=cart2pol(P(1,:),P(2,:));
Uex=zeros(size(R));
nbes=1;
Uex=besselh(0,nbes,k*R) * C(1)/besselh(0,nbes,k);
for i=2:Nc, 
  n=i-1;  
  Uex = Uex + besselh(n,nbes,k*R) * C(i)/besselh(n,nbes,k) .* exp(1i*n*T);
  Uex = Uex + besselh(-n,nbes,k*R) * C(N-(n-1))/besselh(-n,nbes,k) .* exp(-1i*n*T);
end;


disp('the error won''t be less than ~1e-2 as long as a first order absorbing boundary condition will be used');
disp(sprintf('rel error ||Uex-U||_inf=%g',max(abs(Ud-Uex))/max(abs(Uex))));
disp(sprintf('rel error ||Uex-U||_L2=%g',...
             gf_compute(mfd,Uex-Ud,'L2 norm',mim)/gf_compute(mfd,Uex,'L2 norm',mim)));
disp(sprintf('rel error ||Uex-U||_H1=%g',...
             gf_compute(mfd,Uex-Ud,'H1 norm',mim)/gf_compute(mfd,Uex,'H1 norm',mim)));

% adjust the 'refine' parameter to enhance the quality of the picture
gf_plot(mfu,real(U(:)'),'mesh','on','refine',8); 
