size_t filip_n = 82;
size_t filip_p = 11;

double filip_x[] = { -6.860120914, -4.324130045, -4.358625055,
-4.358426747, -6.955852379, -6.661145254, -6.355462942, -6.118102026,
-7.115148017, -6.815308569, -6.519993057, -6.204119983, -5.853871964,
-6.109523091, -5.79832982, -5.482672118, -5.171791386, -4.851705903,
-4.517126416, -4.143573228, -3.709075441, -3.499489089, -6.300769497,
-5.953504836, -5.642065153, -5.031376979, -4.680685696, -4.329846955,
-3.928486195, -8.56735134, -8.363211311, -8.107682739, -7.823908741,
-7.522878745, -7.218819279, -6.920818754, -6.628932138, -6.323946875,
-5.991399828, -8.781464495, -8.663140179, -8.473531488, -8.247337057,
-7.971428747, -7.676129393, -7.352812702, -7.072065318, -6.774174009,
-6.478861916, -6.159517513, -6.835647144, -6.53165267, -6.224098421,
-5.910094889, -5.598599459, -5.290645224, -4.974284616, -4.64454848,
-4.290560426, -3.885055584, -3.408378962, -3.13200249, -8.726767166,
-8.66695597, -8.511026475, -8.165388579, -7.886056648, -7.588043762,
-7.283412422, -6.995678626, -6.691862621, -6.392544977, -6.067374056,
-6.684029655, -6.378719832, -6.065855188, -5.752272167, -5.132414673,
-4.811352704, -4.098269308, -3.66174277, -3.2644011};

double filip_y[] = { 0.8116, 0.9072, 0.9052, 0.9039, 0.8053, 0.8377,
0.8667, 0.8809, 0.7975, 0.8162, 0.8515, 0.8766, 0.8885, 0.8859,
0.8959, 0.8913, 0.8959, 0.8971, 0.9021, 0.909, 0.9139, 0.9199, 0.8692,
0.8872, 0.89, 0.891, 0.8977, 0.9035, 0.9078, 0.7675, 0.7705, 0.7713,
0.7736, 0.7775, 0.7841, 0.7971, 0.8329, 0.8641, 0.8804, 0.7668,
0.7633, 0.7678, 0.7697, 0.77, 0.7749, 0.7796, 0.7897, 0.8131, 0.8498,
0.8741, 0.8061, 0.846, 0.8751, 0.8856, 0.8919, 0.8934, 0.894, 0.8957,
0.9047, 0.9129, 0.9209, 0.9219, 0.7739, 0.7681, 0.7665, 0.7703,
0.7702, 0.7761, 0.7809, 0.7961, 0.8253, 0.8602, 0.8809, 0.8301,
0.8664, 0.8834, 0.8898, 0.8964, 0.8963, 0.9074, 0.9119, 0.9228 } ;


void
test_filip ()
{
  size_t i, j;
  {
    gsl_multifit_linear_workspace * work = 
      gsl_multifit_linear_alloc (filip_n, filip_p);

    gsl_matrix * X = gsl_matrix_alloc (filip_n, filip_p);
    gsl_vector_view y = gsl_vector_view_array (filip_y, filip_n);
    gsl_vector * c = gsl_vector_alloc (filip_p);
    gsl_matrix * cov = gsl_matrix_alloc (filip_p, filip_p);
    gsl_vector * r = gsl_vector_alloc(filip_n);
    gsl_vector_view diag;

    double chisq;

    double expected_c[11] = { -1467.48961422980,      
                              -2772.17959193342,      
                              -2316.37108160893,      
                              -1127.97394098372,      
                              -354.478233703349,      
                              -75.1242017393757,      
                              -10.8753180355343,      
                              -1.06221498588947,      
                              -0.670191154593408E-01, 
                              -0.246781078275479E-02, 
                              -0.402962525080404E-04 };

    double expected_sd[11]  = { 298.084530995537,     
                               559.779865474950,     
                               466.477572127796,     
                               227.204274477751,     
                               71.6478660875927,     
                               15.2897178747400,     
                               2.23691159816033,     
                               0.221624321934227,    
                               0.142363763154724E-01,
                               0.535617408889821E-03,
                               0.896632837373868E-05 };

    double expected_chisq = 0.795851382172941E-03;

    for (i = 0 ; i < filip_n; i++) 
      {
        for (j = 0; j < filip_p; j++) 
          {
            gsl_matrix_set(X, i, j, pow(filip_x[i], j));
          }
      }

    gsl_multifit_linear (X, &y.vector, c, cov, &chisq, work);

    gsl_test_rel (gsl_vector_get(c,0), expected_c[0], 1e-7, "filip gsl_fit_multilinear c0") ;
    gsl_test_rel (gsl_vector_get(c,1), expected_c[1], 1e-7, "filip gsl_fit_multilinear c1") ;
    gsl_test_rel (gsl_vector_get(c,2), expected_c[2], 1e-7, "filip gsl_fit_multilinear c2") ;
    gsl_test_rel (gsl_vector_get(c,3), expected_c[3], 1e-7, "filip gsl_fit_multilinear c3") ;
    gsl_test_rel (gsl_vector_get(c,4), expected_c[4], 1e-7, "filip gsl_fit_multilinear c4") ;
    gsl_test_rel (gsl_vector_get(c,5), expected_c[5], 1e-7, "filip gsl_fit_multilinear c5") ;
    gsl_test_rel (gsl_vector_get(c,6), expected_c[6], 1e-7, "filip gsl_fit_multilinear c6") ;
    gsl_test_rel (gsl_vector_get(c,7), expected_c[7], 1e-7, "filip gsl_fit_multilinear c7") ;
    gsl_test_rel (gsl_vector_get(c,8), expected_c[8], 1e-7, "filip gsl_fit_multilinear c8") ;
    gsl_test_rel (gsl_vector_get(c,9), expected_c[9], 1e-7, "filip gsl_fit_multilinear c9") ;
    gsl_test_rel (gsl_vector_get(c,10), expected_c[10], 1e-7, "filip gsl_fit_multilinear c10") ;

    diag = gsl_matrix_diagonal (cov);

    gsl_test_rel (gsl_vector_get(&diag.vector,0), pow(expected_sd[0],2.0), 1e-6, "filip gsl_fit_multilinear cov00") ;
    gsl_test_rel (gsl_vector_get(&diag.vector,1), pow(expected_sd[1],2.0), 1e-6, "filip gsl_fit_multilinear cov11") ;
    gsl_test_rel (gsl_vector_get(&diag.vector,2), pow(expected_sd[2],2.0), 1e-6, "filip gsl_fit_multilinear cov22") ;
    gsl_test_rel (gsl_vector_get(&diag.vector,3), pow(expected_sd[3],2.0), 1e-6, "filip gsl_fit_multilinear cov33") ;
    gsl_test_rel (gsl_vector_get(&diag.vector,4), pow(expected_sd[4],2.0), 1e-6, "filip gsl_fit_multilinear cov44") ;
    gsl_test_rel (gsl_vector_get(&diag.vector,5), pow(expected_sd[5],2.0), 1e-6, "filip gsl_fit_multilinear cov55") ;
    gsl_test_rel (gsl_vector_get(&diag.vector,6), pow(expected_sd[6],2.0), 1e-6, "filip gsl_fit_multilinear cov66") ;
    gsl_test_rel (gsl_vector_get(&diag.vector,7), pow(expected_sd[7],2.0), 1e-6, "filip gsl_fit_multilinear cov77") ;
    gsl_test_rel (gsl_vector_get(&diag.vector,8), pow(expected_sd[8],2.0), 1e-6, "filip gsl_fit_multilinear cov88") ;
    gsl_test_rel (gsl_vector_get(&diag.vector,9), pow(expected_sd[9],2.0), 1e-6, "filip gsl_fit_multilinear cov99") ;
    gsl_test_rel (gsl_vector_get(&diag.vector,10), pow(expected_sd[10],2.0), 1e-6, "filip gsl_fit_multilinear cov1010") ;

    gsl_test_rel (chisq, expected_chisq, 1e-7, "filip gsl_fit_multilinear chisq") ;

    gsl_multifit_linear_residuals(X, &y.vector, c, r);
    gsl_blas_ddot(r, r, &chisq);
    gsl_test_rel (chisq, expected_chisq, 1e-7, "filip gsl_fit_multilinear residuals") ;

    gsl_vector_free(c);
    gsl_matrix_free(cov);
    gsl_matrix_free(X);
    gsl_vector_free(r);
    gsl_multifit_linear_free (work);
  }

  {
    gsl_multifit_linear_workspace * work = 
      gsl_multifit_linear_alloc (filip_n, filip_p);

    gsl_matrix * X = gsl_matrix_alloc (filip_n, filip_p);
    gsl_vector_view y = gsl_vector_view_array (filip_y, filip_n);
    gsl_vector * w = gsl_vector_alloc (filip_n);
    gsl_vector * c = gsl_vector_alloc (filip_p);
    gsl_vector * r = gsl_vector_alloc (filip_n);
    gsl_matrix * cov = gsl_matrix_alloc (filip_p, filip_p);

    double chisq;

    double expected_c[11] = { -1467.48961422980,      
                              -2772.17959193342,      
                              -2316.37108160893,      
                              -1127.97394098372,      
                              -354.478233703349,      
                              -75.1242017393757,      
                              -10.8753180355343,      
                              -1.06221498588947,      
                              -0.670191154593408E-01, 
                              -0.246781078275479E-02, 
                              -0.402962525080404E-04 };

    /* computed using GNU Calc */

    double expected_cov[11][11] ={ {  7.9269341767252183262588583867942e9,  1.4880416622254098343441063389706e10, 1.2385811858111487905481427591107e10, 6.0210784406215266653697715794241e9, 1.8936652526181982747116667336389e9, 4.0274900618493109653998118587093e8, 5.8685468011819735806180092394606e7, 5.7873451475721689084330083708901e6,  3.6982719848703747920663262917032e5,  1.3834818802741350637527054170891e4,   2.301758578713219280719633494302e2  },
      { 1.4880416622254098334697515488559e10, 2.7955091668548290835529555438088e10, 2.3286604504243362691678565997033e10, 1.132895006796272983689297219686e10, 3.5657281653312473123348357644683e9, 7.5893300392314445528176646366087e8, 1.1066654886143524811964131660002e8, 1.0921285448484575110763947787775e7,  6.9838139975394769253353547606971e5,  2.6143091775349597218939272614126e4,  4.3523386330348588614289505633539e2  },
      { 1.2385811858111487890788272968677e10, 2.3286604504243362677757802422747e10, 1.9412787917766676553608636489674e10, 9.4516246492862131849077729250098e9, 2.9771226694709917550143152097252e9, 6.3413035086730038062129508949859e8, 9.2536164488309401636559552742339e7, 9.1386304643423333815338760248027e6,  5.8479478338916429826337004060941e5,  2.1905933113294737443808429764554e4,  3.6493161325305557266196635180155e2  },
      { 6.0210784406215266545770691532365e9,  1.1328950067962729823273441573365e10, 9.4516246492862131792040001429636e9,  4.6053152992000107509329772255094e9, 1.4517147860312147098138030287038e9, 3.0944988323328589376402579060072e8, 4.5190223822292688669369522708712e7, 4.4660958693678497534529855690752e6,  2.8599340736122198213681258676423e5,  1.0720394998549386596165641244705e4,  1.7870937745661967319298031044424e2  },
      { 1.8936652526181982701620450132636e9,  3.5657281653312473058825073094524e9,  2.9771226694709917514149924058297e9,  1.451714786031214708936087401632e9,  4.5796563896564815123074920050827e8, 9.7693972414561515534525103622773e7, 1.427717861635658545863942948444e7,  1.4120161287735817621354292900338e6,  9.0484361228623960006818614875557e4,   3.394106783764852373199087455398e3,  5.6617406468519495376287407526295e1  },
    { 4.0274900618493109532650887473599e8,   7.589330039231444534478894935778e8,  6.3413035086730037947153564986653e8,   3.09449883233285893390542947998e8,  9.7693972414561515475770399055121e7, 2.0855726248311948992114244257719e7, 3.0501263034740400533872858749566e6, 3.0187475839310308153394428784224e5,  1.9358204633534233524477930175632e4,  7.2662989867560017077361942813911e2,  1.2129002231061036467607394277965e1  },
      {  5.868546801181973559370854830868e7,  1.1066654886143524778548044386795e8,  9.2536164488309401413296494869777e7,  4.5190223822292688587853853162072e7, 1.4277178616356585441556046753562e7, 3.050126303474040051574715592746e6,  4.4639982579046340884744460329946e5, 4.4212093985989836047285007760238e4,  2.8371395028774486687625333589972e3,  1.0656694507620102300567296504381e2,  1.7799982046359973175080475654123e0  },
      { 5.7873451475721688839974153925406e6,  1.0921285448484575071271480643397e7,  9.1386304643423333540728480344578e6,  4.4660958693678497427674903565664e6, 1.4120161287735817596182229182587e6, 3.0187475839310308117812257613082e5, 4.4212093985989836021482392757677e4, 4.3818874017028389517560906916315e3,   2.813828775753142855163154605027e2,  1.0576188138416671883232607188969e1,  1.7676976288918295012452853715408e-1 },
      { 3.6982719848703747742568351456818e5,  6.9838139975394768959780068745979e5,  5.8479478338916429616547638954781e5,  2.8599340736122198128717796825489e5, 9.0484361228623959793493985226792e4, 1.9358204633534233490579641064343e4, 2.8371395028774486654873647731797e3, 2.8138287757531428535592907878017e2,  1.8081118503579798222896804627964e1,  6.8005074291434681866415478598732e-1, 1.1373581557749643543869665860719e-2 },
      { 1.3834818802741350562839757244708e4,   2.614309177534959709397445440919e4,  2.1905933113294737352721470167247e4,  1.0720394998549386558251721913182e4, 3.3941067837648523632905604575131e3, 7.2662989867560016909534954790835e2, 1.0656694507620102282337905013451e2, 1.0576188138416671871337685672492e1,  6.8005074291434681828743281967838e-1, 2.5593857187900736057022477529078e-2, 4.2831487599116264442963102045936e-4 },
      { 2.3017585787132192669801658674163e2,  4.3523386330348588381716460685124e2,  3.6493161325305557094116270974735e2,  1.7870937745661967246233792737255e2, 5.6617406468519495180024059284629e1, 1.2129002231061036433003571679329e1, 1.7799982046359973135014027410646e0, 1.7676976288918294983059118597214e-1, 1.137358155774964353146460100337e-2,  4.283148759911626442000316269063e-4,  7.172253875245080423800933453952e-6  } };

    double expected_chisq = 0.795851382172941E-03;

    for (i = 0 ; i < filip_n; i++) 
      {
        for (j = 0; j < filip_p; j++) 
          {
            gsl_matrix_set(X, i, j, pow(filip_x[i], j));
          }
      }

    gsl_vector_set_all (w, 1.0);

    gsl_multifit_wlinear (X, w, &y.vector, c, cov, &chisq, work);

    gsl_test_rel (gsl_vector_get(c,0), expected_c[0], 1e-7, "filip gsl_fit_multilinear c0") ;
    gsl_test_rel (gsl_vector_get(c,1), expected_c[1], 1e-7, "filip gsl_fit_multilinear c1") ;
    gsl_test_rel (gsl_vector_get(c,2), expected_c[2], 1e-7, "filip gsl_fit_multilinear c2") ;
    gsl_test_rel (gsl_vector_get(c,3), expected_c[3], 1e-7, "filip gsl_fit_multilinear c3") ;
    gsl_test_rel (gsl_vector_get(c,4), expected_c[4], 1e-7, "filip gsl_fit_multilinear c4") ;
    gsl_test_rel (gsl_vector_get(c,5), expected_c[5], 1e-7, "filip gsl_fit_multilinear c5") ;
    gsl_test_rel (gsl_vector_get(c,6), expected_c[6], 1e-7, "filip gsl_fit_multilinear c6") ;
    gsl_test_rel (gsl_vector_get(c,7), expected_c[7], 1e-7, "filip gsl_fit_multilinear c7") ;
    gsl_test_rel (gsl_vector_get(c,8), expected_c[8], 1e-7, "filip gsl_fit_multilinear c8") ;
    gsl_test_rel (gsl_vector_get(c,9), expected_c[9], 1e-7, "filip gsl_fit_multilinear c9") ;
    gsl_test_rel (gsl_vector_get(c,10), expected_c[10], 1e-7, "filip gsl_fit_multilinear c10") ;


    for (i = 0; i < filip_p; i++) 
      {
        for (j = 0; j < filip_p; j++)
          {
            gsl_test_rel (gsl_matrix_get(cov,i,j), expected_cov[i][j], 1e-6,
                          "filip gsl_fit_wmultilinear cov(%d,%d)", i, j) ;
          }
      }

    gsl_test_rel (chisq, expected_chisq, 1e-7, "filip gsl_fit_wmultilinear chisq") ;

    gsl_multifit_linear_residuals(X, &y.vector, c, r);
    gsl_blas_ddot(r, r, &chisq);
    gsl_test_rel (chisq, expected_chisq, 1e-7, "filip gsl_fit_wmultilinear residuals") ;

    gsl_vector_free(w);
    gsl_vector_free(c);
    gsl_vector_free(r);
    gsl_matrix_free(cov);
    gsl_matrix_free(X);
    gsl_multifit_linear_free (work);
  }
}
