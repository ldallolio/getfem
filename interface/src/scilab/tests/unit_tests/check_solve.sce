// check: gf_mesh('cartesian_mesh'), gf_mesh('triangles grid'), 
// gf_compute('interpolate on Q1 grid')
// gf_compute('gradient'), gf_mesh_fem_set('boundary'), gf_solve(stokes)
// gf_compute('interpolate on'), discontinuous fems, L2&H1 norms
tic;
gf_workspace('clear all');
pde = init_pde();
m1 = gf_mesh('empty',1);
pde('type')   = 'stokes';
pde('viscos') = 1.0;
pde = add_empty_bound(pde);
pde('bound')($)('type') = 'Dirichlet';
pde('bound')($)('R')    = list('-y.*(y-1)',0);
pde('bound')($)('H')    = [];
m = gf_mesh('cartesian',[0:.3:4.8],[0:.2:1]);
pde('mf_u') = gf_mesh_fem(m,2);
pde('mf_p') = gf_mesh_fem(m,1);
pde('mf_d') = gf_mesh_fem(m,1);
pde('mim')  = gf_mesh_im(m, gf_integ('IM_EXACT_PARALLELEPIPED(2)'));
mf_comp = gf_mesh_fem(m,2);
gf_mesh_fem_set(pde('mf_u'),'fem',gf_fem('FEM_QK(2,2)'));
gf_mesh_fem_set(mf_comp ,'fem',gf_fem('FEM_QK(2,2)'));
// the piecewise linear mf_d will induce a small error on the solution
// since the dirichlet condition is parabolic
gf_mesh_fem_set(pde('mf_d'),'fem',gf_fem('FEM_QK(2,1)'));
gf_mesh_fem_set(pde('mf_p'),'fem',gf_fem('FEM_QK(2,1)'));
all_faces = gf_mesh_get(m, 'outer faces', gf_mesh_get(m, 'cvid'));
for mf = list(pde('mf_u'), pde('mf_p'), pde('mf_d'))
  gf_mesh_set(m, 'boundary', 1, all_faces); // YC: ???
end
[U,P] = gf_solve(pde);  
Uco = gf_compute(pde('mf_u'), U, 'interpolate on', mf_comp); // tests interpolation on same mesh
dof = gf_mesh_fem_get(mf_comp, 'basic dof nodes'); 
Xc  = dof(1,1:2:$); 
Yc  = dof(2,1:2:$);
Uex = [-Yc.*(Yc-1); zeros(1,length(Xc))];
Uex = Uex(:)';
// norm tests
l2m = gf_compute(mf_comp,Uex,'L2 norm',pde('mim'));
gfassert('abs(l2m-.4)<1e-10');
printf('L2 norm %f',l2m);
l2err = gf_compute(mf_comp,Uex-Uco,'L2 norm',pde('mim'));
printf('L2 err %f', l2err);
h1err = gf_compute(mf_comp,Uex-Uco,'H1 norm',pde('mim'));
printf('H1 err %f', h1err);
printf('done in %.2f sec.',toc());

h = gcf();
h.color_map = jetcolormap(255);
drawlater;
gf_plot(mf_comp,Uex-Uco,'quiver','on','quiver_scale',0.5); 
//gf_plot(mf_comp,Uex-Uco,'norm','on','quiver','on','quiver_scale',0.1); 
colorbar(min(Uex-Uco),max(Uex-Uco));
drawnow; 
gfassert('abs(l2err)<0.016'); // 0.015926
gfassert('abs(h1err)<0.0665'); // 0.066215
//[Uq,Iq,mfq] = gf_compute(pde('mf_u'), U, 'interpolate on Q1 grid', 'regular h', [.05, .05]);
[Uq,Iq,mfq] = gf_compute_Q1grid_interp(pde('mf_u'), U, 'regular h', [.05, .05]);
[XX,YY] = meshgrid(0:.05:4.8,0.:0.05:1);  XX=XX'; YY=YY';
UU = -YY.*(YY-1);
gfassert('max(max(abs(UU-squeeze(Uq(1,:,:))))) < 0.01001');
gfassert('max(max(abs(Uq(2,:,:)))) < 0.0030');
//gradient check
mtri  = gf_mesh('triangles grid',[0 .2 .4 .8 1:.3:4.8],[0:.2:.6 .9 1]);
mf_DU = gf_mesh_fem(mtri,2);
gf_mesh_fem_set(mf_DU,'fem',gf_fem('FEM_PK_DISCONTINUOUS(2,1)'));
mim2  = gf_mesh_im(mtri,2);
gfasserterr('gf_compute(pde(''mf_u''), U, ''gradient'', mf_DU)'); // cause different meshes;
UU   = gf_compute(pde('mf_u'), U, 'interpolate on', mf_DU); // tests interpolation on other mesh
DU   = gf_compute(mf_DU, UU, 'gradient', mf_DU);
dof  = gf_mesh_fem_get(mf_DU, 'basic dof nodes'); Xc=dof(1,1:2:$); Yc=dof(2,1:2:$);
DUex = [1-2*Yc; zeros(1,length(Xc))]; 
DUex = DUex(:)';
_diff = norm(DUex-DU(2,:));
gfassert('_diff>4.62 & _diff<4.64');
// yes the error on the derivative is quite big. This is because we interpolated
// U on mf_DU which is piecewise linear the 3 plots below illustrate this
h = scf();
h.color_map = jetcolormap(255);
drawlater;
subplot(3,1,1); 
gf_plot(mf_DU, DU(2,:),'mesh','x'); 
colorbar(min(DU(2,:)),max(DU(2,:))); //dUx/dy
subplot(3,1,2); 
gf_plot(mf_DU, DUex,'mesh','x'); 
colorbar(min(DUex),max(DUex));
subplot(3,1,3); 
gf_plot(mf_DU, DUex-DU(2,:),'mesh','x'); 
colorbar(min(DUex-DU(2,:)),max(DUex-DU(2,:)));    
drawnow;
d2 = gf_compute(mf_DU,DUex-DU(2,:),'L2 norm',mim2);
gfassert('d2>0.28 & d2 < 0.29'); // 0.2866
//gradient of vector fields
DU2 = gf_compute(mf_DU, [UU;2*UU;3*UU], 'gradient', mf_DU);
d   = permute(cat(3,DU,2*DU,3*DU),[1,3,2]);
//d(1:10)
//DU2(1:10)
gfassert('max(abs(DU2(:)-d(:))) < 2e-15');