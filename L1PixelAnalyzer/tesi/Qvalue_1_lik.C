void Qvalue_1_lik (TString v, TString sel ) 
{

  const int nvars=1;
  TString var[1] = v;

  const int nbins=50;
  const int nbinsl=20;
  TString pippo[nvars];
  TString pippotot[nvars];
  TString pippotth[nvars];
  TString pippototS[nvars];
  TString pippotthS[nvars];
  for ( int ivar=0; ivar<nvars; ivar++ ) { 
    pippo[ivar] = var[ivar]+sel; 
    pippotot[ivar]=var[ivar]+sel+"_bgr";
    pippotth[ivar]=var[ivar]+sel+"_sig";
    pippototS[ivar]=var[ivar]+sel+"_bgrS";
    pippotthS[ivar]=var[ivar]+sel+"_sigS";
  }

  TString fname;
  fname="./root/"+v+sel+".root";
  TFile * Smoothed = new TFile(fname);
  Smoothed->cd();

  TCanvas * b = new TCanvas ("b", "Cut optimization", 500, 500 );
  b->Divide(2,2);
  for ( int ivar=0; ivar<nvars; ivar++ ) {

    TH1D * Histo_TTHS = dynamic_cast<TH1D*>(Smoothed->Get(pippotthS[ivar]));
    TH1D * Histo_TOTS = dynamic_cast<TH1D*>(Smoothed->Get(pippototS[ivar]));
    double minx=Histo_TTHS->GetBinLowEdge(1);
    double maxx=nbins*Histo_TTHS->GetBinWidth(1)+minx;
    
    ////////////////////////////////////////////////////////
    // Check of Likelihood ratio per each variable
    // -------------------------------------------
    TH1D * LS = new TH1D ( "LS", "LS", nbinsl, -2., 2. );
    TH1D * LB = new TH1D ( "LB", "LB", nbinsl, -2., 2. );
    LS->Sumw2();
    LB->Sumw2();
    for ( int i=0; i<10000; i++ ) {
      // Fish at random from bgr shape
      double x = Histo_TOTS->GetRandom();
      int bx;
      double r;
      bx= (int)((x-minx)/(maxx-minx)*nbins)+1;
      if ( bx>=1 && bx<=nbins ) {
	double fs = Histo_TTHS->GetBinContent(bx);
	double fb = Histo_TOTS->GetBinContent(bx);
	if (fb>0 && fs>0 ) {
	  r = fs/fb;
	} else {
	  if ( fb==0. ) cout << "problem with getrandom" << endl;
	  if ( fs==0. ) r = 0.01; 
	}
      }
      double rel_lik;
      if ( r>=0 ) {
	rel_lik = log(r);
      } else {
	rel_lik=-1.99;	    
      }
      if ( rel_lik<-1.99 ) rel_lik=-1.99;
      if ( rel_lik>1.99 ) rel_lik=1.99;
      LB->Fill(rel_lik);
    }
    for ( int i=0; i<10000; i++ ) {
      // Fish at random from sig shape
      double x = Histo_TTHS->GetRandom();
      int bx;
      double r;
      bx= (int)((x-minx)/(maxx-minx)*nbins)+1;
      if ( bx>=1 && bx<=nbins ) {
	double fs = Histo_TTHS->GetBinContent(bx);
	double fb = Histo_TOTS->GetBinContent(bx);
	if (fb>0 && fs>0 ) {
	  r = fs/fb;
	} else {
	  if ( fs==0 ) cout << "problem with getrandom" << endl;
	  if ( fb==0 ) r = 100.; 
	}
      }
      double rel_lik;
      if ( r>=0 ) {
	rel_lik = log(r);
      } else {
	rel_lik=-1.99;	    
      }
      if ( rel_lik<-1.99 ) rel_lik=-1.99;
      if ( rel_lik>1.99 ) rel_lik=1.99;
      LS->Fill(rel_lik);
    }

    ////////////////////////////////////////////////////////

    // Now compute max quality factor for this distribution
    // -----------------------------------------------------
    TH1D * S_  = new TH1D ( "S",  "S "+var[ivar],  nbinsl, -2., 2. );     // Signal distribution
    TH1D * B_  = new TH1D ( "B",  "B "+var[ivar],  nbinsl, -2., 2. );     // Total background distribution
    TH1D * ES_ = new TH1D ( "ES", "ES "+var[ivar], nbinsl, -2., 2. );  // Efficiency of signal 
    TH1D * EB_ = new TH1D ( "EB", "EB "+var[ivar], nbinsl, -2., 2. );   // Efficiency of background
    TH1D * R_  = new TH1D ( "R",  "R "+var[ivar],  nbinsl, -2., 2. );     // Esig/Ebgr distribution
    TH1D * Q_  = new TH1D ( "Q",  "Q "+var[ivar],  nbinsl, -2., 2. );     // Q-value
    
    // First of all, fill normalized distributions
    // -------------------------------------------
    double IS = LS->Integral();
    double IB = LB->Integral();
    double maxS=0;
    double maxB=0;
    for ( int ibin=1; ibin<=nbinsl; ibin++ ) {
      double cont_tth = LS->GetBinContent(ibin);
      double erro_tth = LS->GetBinError(ibin);
      S_->SetBinContent(ibin,cont_tth/IS);
      S_->SetBinError(ibin,erro_tth/IS);
      if ( maxS<cont_tth/IS ) maxS=cont_tth/IS;
      double cont_tot = LB->GetBinContent(ibin);
      double erro_tot = LB->GetBinError(ibin);
      B_->SetBinContent(ibin,cont_tot/IB);
      B_->SetBinError(ibin,erro_tot/IB);
      if ( maxB<cont_tot/IB ) maxB=cont_tot/IB;
    }

    ////////////////////////////////////////////
    // Efficiency and gains for x>xcut 
    // -------------------------------
    double es[nbinsl];
    double s_es[nbinsl];
    double eb[nbinsl];
    double s_eb[nbinsl];
    for ( int ibin=1; ibin<=nbinsl; ibin++ ) {

      // Get signal efficiency curve
      // ---------------------------
      double fs=0.;    // signal failing cut
      double s2_fs=0.;
      double ps=0.;    // signal passing cut
      double s2_ps=0.;
      for ( int jbin=1; jbin<=nbinsl; jbin++ ) {
	if ( jbin<ibin ) { 
	  fs    += S_->GetBinContent(jbin);
	  s2_fs += pow(S_->GetBinError(jbin),2);
	}
	if ( jbin>=ibin ) {
	  ps    += S_->GetBinContent(jbin);
	  s2_ps += pow(S_->GetBinError(jbin),2);
	}
      }
      if ( fs+ps>0. ) {
	es[ibin-1]=ps/(fs+ps);
	s_es[ibin-1]=sqrt(ps*ps*s2_fs+fs*fs*s2_ps)/pow(fs+ps,2);
      } else {
	es[ibin-1]=0;
	s_es[ibin-1]=0;
      }
      ES_->SetBinContent(ibin,es[ibin-1]);
      ES_->SetBinError(ibin,s_es[ibin-1]);

      // Get background efficiency curve
      // -------------------------------
      double fb=0.;    // background failing cut
      double s2_fb=0.;
      double pb=0.;    // background passing cut
      double s2_pb=0.;
      for ( int jbin=1; jbin<=nbinsl; jbin++ ) {
	if ( jbin<ibin ) { 
	  fb    += B_->GetBinContent(jbin);
	  s2_fb += pow(B_->GetBinError(jbin),2);
	}
	if ( jbin>=ibin ) {
	  pb    += B_->GetBinContent(jbin);
	  s2_pb += pow(B_->GetBinError(jbin),2);
	}
      }
      if ( fb+pb>0 ) {
	eb[ibin-1]=pb/(fb+pb);
	s_eb[ibin-1]=sqrt(pb*pb*s2_fb+fb*fb*s2_pb)/pow(fb+pb,2);
      } else {
	eb[ibin-1]=0.;
	s_eb[ibin-1]=0.;
      }
      EB_->SetBinContent(ibin,eb[ibin-1]);
      EB_->SetBinError(ibin,s_eb[ibin-1]);
    }

    // Now compute efficiency ratio and quality factor
    // -----------------------------------------------
    double maxQm1s=1.;
    double s_maxQm1s=0.1;
    double x_maxQ=0;
    for ( int ibin=1; ibin<=nbinsl; ibin++ ) {
      double R;
      double s_R;
      double Q=1.; 
      double s_Q=0.1;
      if ( eb[ibin-1]>0. ) {
	R = es[ibin-1]/eb[ibin-1];
	s_R = sqrt(s_es[ibin-1]*s_es[ibin-1]/eb[ibin-1]/eb[ibin-1]+
		   es[ibin-1]*es[ibin-1]*s_eb[ibin-1]*s_eb[ibin-1]/pow(eb[ibin-1],4));
	Q = es[ibin-1]*es[ibin-1]/eb[ibin-1];
	s_Q = sqrt(pow(2*es[ibin-1]/eb[ibin-1]*s_es[ibin-1],2)+
		   pow(es[ibin-1]/eb[ibin-1],4)*pow(s_eb[ibin-1],2));
	R_->SetBinContent(ibin,R);
	R_->SetBinError(ibin,s_R);
	Q_->SetBinContent(ibin,Q);
	Q_->SetBinError(ibin,s_Q);
	if ( maxQm1s<Q-s_Q && (maxQm1s-1.)/s_maxQm1s<(Q-1.)/s_Q ) {
	  maxQm1s=Q;
	  s_maxQm1s=s_Q;
	  x_maxQ=(double)ibin*4/(double)nbinsl;
	} 
      } 
    }
    double maxX=maxS;
    if (maxB>maxX) maxX=maxB;
    cout << var[ivar] << " : maximum quality factor " << maxQm1s << "+-" << s_maxQm1s 
	 << " at LR > " << x_maxQ << endl; 
    
    b->cd(1);
    S_->SetMaximum(1.2*maxX);
    S_->SetLineColor(kRed);
    S_->SetMarkerColor(kRed);
    S_->DrawCopy("PE");
    B_->SetLineColor(kBlue);
    B_->SetMarkerColor(kBlue);
    B_->DrawCopy("PESAME");
    b->cd(2);
    ES_->SetLineColor(kRed);
    ES_->SetMarkerColor(kRed);
    ES_->DrawCopy("PE");
    EB_->SetLineColor(kBlue);
    EB_->SetMarkerColor(kBlue);
    EB_->DrawCopy("PESAME");
    b->cd(3);
    R_->DrawCopy("PE");
    b->cd(4);
    Q_->DrawCopy("PE");
    b->Print("./ps/"+pippo[ivar]+"_opt_L.ps");

    delete S_;
    delete B_;
    delete ES_;
    delete EB_;
    delete R_;
    delete Q_;

    delete Histo_TTHS;
    delete Histo_TOTS;
    delete LS;
    delete LB;

  } // end ivar loop

}
