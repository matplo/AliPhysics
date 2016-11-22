//
// Creation date: 2016/09/06
// Author: Ionut-Cristian Arsene, iarsene@cern.ch, i.c.arsene@fys.uio.no

#include "AliReducedAnalysisJpsi2ee.h"

#include <iostream>
using std::cout;
using std::endl;

#include <TClonesArray.h>
#include <TIterator.h>

#include "AliReducedVarManager.h"
#include "AliReducedEventInfo.h"
#include "AliReducedBaseEvent.h"
#include "AliReducedBaseTrack.h"
#include "AliReducedTrackInfo.h"
#include "AliReducedPairInfo.h"
#include "AliHistogramManager.h"

ClassImp(AliReducedAnalysisJpsi2ee);


//___________________________________________________________________________
AliReducedAnalysisJpsi2ee::AliReducedAnalysisJpsi2ee() :
  AliReducedAnalysisTaskSE(),
  fHistosManager(new AliHistogramManager("Histogram Manager", AliReducedVarManager::kNVars)),
  fMixingHandler(new AliMixingHandler()),
  fOptionRunMixing(kTRUE),
  fEventCuts(),
  fTrackCuts(),
  fPreFilterTrackCuts(),
  fPairCuts(),
  fPreFilterPairCuts(),
  fPosTracks(),
  fNegTracks(),
  fPrefilterPosTracks(),
  fPrefilterNegTracks(),
  fEventCounter(0)
{
  //
  // default constructor
  //
}


//___________________________________________________________________________
AliReducedAnalysisJpsi2ee::AliReducedAnalysisJpsi2ee(const Char_t* name, const Char_t* title) :
  AliReducedAnalysisTaskSE(name,title),
  fHistosManager(new AliHistogramManager("Histogram Manager", AliReducedVarManager::kNVars)),
  fMixingHandler(new AliMixingHandler()),
  fOptionRunMixing(kTRUE),
  fEventCuts(),
  fTrackCuts(),
  fPreFilterTrackCuts(),
  fPairCuts(),
  fPreFilterPairCuts(),
  fPosTracks(),
  fNegTracks(),
  fPrefilterPosTracks(),
  fPrefilterNegTracks(),
  fEventCounter(0)
{
  //
  // named constructor
  //
   fEventCuts.SetOwner(kTRUE);
   fTrackCuts.SetOwner(kTRUE);
   fPreFilterTrackCuts.SetOwner(kTRUE);
   fPairCuts.SetOwner(kTRUE);
   fPreFilterPairCuts.SetOwner(kTRUE);
   fPosTracks.SetOwner(kFALSE);
   fNegTracks.SetOwner(kFALSE);
   fPrefilterPosTracks.SetOwner(kFALSE);
   fPrefilterNegTracks.SetOwner(kFALSE);
}


//___________________________________________________________________________
AliReducedAnalysisJpsi2ee::~AliReducedAnalysisJpsi2ee() 
{
  //
  // destructor
  //
   fEventCuts.Clear("C"); fTrackCuts.Clear("C"); fPreFilterTrackCuts.Clear("C"); fPreFilterPairCuts.Clear("C"); fPairCuts.Clear("C");
   fPosTracks.Clear("C"); fNegTracks.Clear("C"); fPrefilterPosTracks.Clear("C"); fPrefilterNegTracks.Clear("C");
   if(fHistosManager) delete fHistosManager;
   if(fMixingHandler) delete fMixingHandler;
}


//___________________________________________________________________________
void AliReducedAnalysisJpsi2ee::AddTrackCut(AliReducedInfoCut* cut) {
   //
   // Add a new cut
   //
   fTrackCuts.Add(cut); 
   fMixingHandler->SetNParallelCuts(fMixingHandler->GetNParallelCuts()+1);
   TString histClassNames = fMixingHandler->GetHistClassNames();
   histClassNames += Form("PairMEPP_%s;", cut->GetName());
   histClassNames += Form("PairMEPM_%s;", cut->GetName());
   histClassNames += Form("PairMEMM_%s;", cut->GetName());
   fMixingHandler->SetHistClassNames(histClassNames.Data());
}


//___________________________________________________________________________
Bool_t AliReducedAnalysisJpsi2ee::IsEventSelected(AliReducedBaseEvent* event, Float_t* values/*=0x0*/) {
  //
  // apply event cuts
  //
  if(fEventCuts.GetEntries()==0) return kTRUE;
  // loop over all the cuts and make a logical and between all cuts in the list
  for(Int_t i=0; i<fEventCuts.GetEntries(); ++i) {
    AliReducedInfoCut* cut = (AliReducedInfoCut*)fEventCuts.At(i);
    if(values) { if(!cut->IsSelected(event, values)) return kFALSE; }
    else { if(!cut->IsSelected(event)) return kFALSE; }
  }
  return kTRUE;
}

//___________________________________________________________________________
Bool_t AliReducedAnalysisJpsi2ee::IsTrackSelected(AliReducedBaseTrack* track, Float_t* values/*=0x0*/) {
  //
  // apply event cuts
  //
  if(fTrackCuts.GetEntries()==0) return kTRUE;
  track->ResetFlags();
  
  for(Int_t i=0; i<fTrackCuts.GetEntries(); ++i) {
    AliReducedInfoCut* cut = (AliReducedInfoCut*)fTrackCuts.At(i);
    if(values) { if(cut->IsSelected(track, values)) track->SetFlag(i); }
    else { if(cut->IsSelected(track)) track->SetFlag(i); }
  }
  return (track->GetFlags()>0 ? kTRUE : kFALSE);
}

//___________________________________________________________________________
Bool_t AliReducedAnalysisJpsi2ee::IsTrackPrefilterSelected(AliReducedBaseTrack* track, Float_t* values/*=0x0*/) {
   //
   // apply event cuts
   //
   if(fPreFilterTrackCuts.GetEntries()==0) return kTRUE;
   
   for(Int_t i=0; i<fPreFilterTrackCuts.GetEntries(); ++i) {
      // if there are more cuts specified, we apply an AND on all of them
      AliReducedInfoCut* cut = (AliReducedInfoCut*)fPreFilterTrackCuts.At(i);
      if(values) { if(!cut->IsSelected(track, values)) return kFALSE; }
      else { if(!cut->IsSelected(track)) return kFALSE; }
   }
   return kTRUE;
}

//___________________________________________________________________________
Bool_t AliReducedAnalysisJpsi2ee::IsPairSelected(Float_t* values) {
  //
  // apply event cuts
  //
  if(fPairCuts.GetEntries()==0) return kTRUE;
  // loop over all the cuts and make a logical and between all cuts in the list
  for(Int_t i=0; i<fPairCuts.GetEntries(); ++i) {
    AliReducedInfoCut* cut = (AliReducedInfoCut*)fPairCuts.At(i);
    if(!cut->IsSelected(values)) return kFALSE;
  }
  return kTRUE;
}

//___________________________________________________________________________
Bool_t AliReducedAnalysisJpsi2ee::IsPairPreFilterSelected(Float_t* values) {
   //
   // apply event cuts
   //
   if(fPreFilterPairCuts.GetEntries()==0) return kTRUE;
   // loop over all the cuts and make a logical and between all cuts in the list
   for(Int_t i=0; i<fPreFilterPairCuts.GetEntries(); ++i) {
      AliReducedInfoCut* cut = (AliReducedInfoCut*)fPreFilterPairCuts.At(i);
      if(!cut->IsSelected(values)) return kFALSE;
   }
   return kTRUE;
}

//___________________________________________________________________________
void AliReducedAnalysisJpsi2ee::Init() {
  //
  // initialize stuff
  //
   AliReducedVarManager::SetDefaultVarNames();
   fHistosManager->SetUseDefaultVariableNames(kTRUE);
   fHistosManager->SetDefaultVarNames(AliReducedVarManager::fgVariableNames,AliReducedVarManager::fgVariableUnits);
   
   fMixingHandler->SetHistogramManager(fHistosManager);
}


//___________________________________________________________________________
void AliReducedAnalysisJpsi2ee::Process() {
  //
  // process the current event
  //  
  if(!fEvent) return;
  AliReducedEventInfo* eventInfo = NULL;
  if(fEvent->IsA()==AliReducedEventInfo::Class()) eventInfo = (AliReducedEventInfo*)fEvent;
  else {
     cout << "ERROR: AliReducedAnalysisJpsi2ee::Process() needs AliReducedEventInfo events" << endl;
     return;
  }
  if(fEventCounter%100000==0) cout << "Event no. " << fEventCounter << endl;
  fEventCounter++;
  
  AliReducedVarManager::SetEvent(fEvent);
  
  // reset the values array, keep only the run wise data (LHC and ALICE GRP information)
  // NOTE: the run wise data will be updated automatically in the VarManager in case a run change is detected
  for(Int_t i=AliReducedVarManager::kNRunWiseVariables; i<AliReducedVarManager::kNVars; ++i) fValues[i]=-9999.;
  
  // fill event information before event cuts
  AliReducedVarManager::FillEventInfo(fEvent, fValues);
  fHistosManager->FillHistClass("Event_BeforeCuts", fValues);
  for(UShort_t ibit=0; ibit<64; ++ibit) {
     AliReducedVarManager::FillEventTagInput(fEvent, ibit, fValues);
     fHistosManager->FillHistClass("EventTag_BeforeCuts", fValues);
  }
  for(UShort_t ibit=0; ibit<64; ++ibit) {
      AliReducedVarManager::FillEventOnlineTrigger(ibit, fValues);
      fHistosManager->FillHistClass("EventTriggers_BeforeCuts", fValues);
  }
  
  
  // apply event selection
  if(!IsEventSelected(fEvent)) return;
  
  // select tracks
  RunTrackSelection();
    
  // Run the prefilter  
  // NOTE: Pair each track from the selected tracks list with all selected tracks in the prefilter track list
  //         If the created pair fails the pair prefilter criteria, then the selected trak is removed from the track list
  //          and further pairing
  //FillTrackHistograms("Track_BeforePrefilter");
  //RunSameEventPairing("PairPrefilterSE");
  RunPrefilter();
  
  fValues[AliReducedVarManager::kNtracksPosAnalyzed] = fPosTracks.GetEntries();
  fValues[AliReducedVarManager::kNtracksNegAnalyzed] = fNegTracks.GetEntries();
  fValues[AliReducedVarManager::kNtracksAnalyzed] = fValues[AliReducedVarManager::kNtracksNegAnalyzed]+fValues[AliReducedVarManager::kNtracksPosAnalyzed];
  
  // Fill track histograms
  FillTrackHistograms();
  
  // Feed the selected tracks to the event mixing handler 
  if(fOptionRunMixing)
    fMixingHandler->FillEvent(&fPosTracks, &fNegTracks, fValues, AliReducedPairInfo::kJpsiToEE);
  
  // Do the same event pairing
  RunSameEventPairing();
 
  // fill event info histograms after cuts
  fHistosManager->FillHistClass("Event_AfterCuts", fValues);
  for(UShort_t ibit=0; ibit<64; ++ibit) {
     AliReducedVarManager::FillEventTagInput(fEvent, ibit, fValues);
     fHistosManager->FillHistClass("EventTag_AfterCuts", fValues);
  }
  for(UShort_t ibit=0; ibit<64; ++ibit) {
     AliReducedVarManager::FillEventOnlineTrigger(ibit, fValues);
     fHistosManager->FillHistClass("EventTriggers_AfterCuts", fValues);
  }
}


//___________________________________________________________________________
void AliReducedAnalysisJpsi2ee::FillTrackHistograms(TString trackClass /*= "Track"*/) {
   //
   // Fill all track histograms
   //
   AliReducedTrackInfo* track=0;
   TIter nextPosTrack(&fPosTracks);
   for(Int_t i=0;i<fPosTracks.GetEntries();++i) {
      track = (AliReducedTrackInfo*)nextPosTrack();
      AliReducedVarManager::FillTrackInfo(track, fValues);
      FillTrackHistograms(track, trackClass);
      //cout << "Pos track " << i << ": "; AliReducedVarManager::PrintBits(track->Status()); cout << endl;
   }
   TIter nextNegTrack(&fNegTracks);
   for(Int_t i=0;i<fNegTracks.GetEntries();++i) {
      track = (AliReducedTrackInfo*)nextNegTrack();
      AliReducedVarManager::FillTrackInfo(track, fValues);
      FillTrackHistograms(track, trackClass);
      //cout << "Neg track " << i << ": "; AliReducedVarManager::PrintBits(track->Status()); cout << endl;
   }
}


//___________________________________________________________________________
void AliReducedAnalysisJpsi2ee::FillTrackHistograms(AliReducedTrackInfo* track, TString trackClass /*="Track"*/) {
   //
   // fill track level histograms
   //
   for(Int_t icut=0; icut<fTrackCuts.GetEntries(); ++icut) {
      if(track->TestFlag(icut)) fHistosManager->FillHistClass(Form("%s_%s", trackClass.Data(), fTrackCuts.At(icut)->GetName()), fValues);
      for(UInt_t iflag=0; iflag<AliReducedVarManager::kNTrackingFlags; ++iflag) {
         AliReducedVarManager::FillTrackingFlag(track, iflag, fValues);
         fHistosManager->FillHistClass(Form("%sStatusFlags_%s", trackClass.Data(), fTrackCuts.At(icut)->GetName()), fValues);
      }
      for(Int_t iLayer=0; iLayer<6; ++iLayer) {
         AliReducedVarManager::FillITSlayerFlag(track, iLayer, fValues);
         fHistosManager->FillHistClass(Form("%sITSclusterMap_%s", trackClass.Data(), fTrackCuts.At(icut)->GetName()), fValues);
      }
      for(Int_t iLayer=0; iLayer<8; ++iLayer) {
         AliReducedVarManager::FillTPCclusterBitFlag(track, iLayer, fValues);
         fHistosManager->FillHistClass(Form("%sTPCclusterMap_%s", trackClass.Data(), fTrackCuts.At(icut)->GetName()), fValues);
      }
   }  // end loop over cuts
}


//___________________________________________________________________________
void AliReducedAnalysisJpsi2ee::FillPairHistograms(ULong_t mask, Int_t pairType, TString pairClass /*="PairSE"*/) {
   //
   // fill pair level histograms
   // NOTE: pairType can be 0,1 or 2 corresponding to ++, +- or -- pairs
   TString typeStr[3] = {"PP", "PM", "MM"};
   for(Int_t icut=0; icut<fTrackCuts.GetEntries(); ++icut) {
      if(mask & (ULong_t(1)<<icut)) fHistosManager->FillHistClass(Form("%s%s_%s", pairClass.Data(), typeStr[pairType].Data(), fTrackCuts.At(icut)->GetName()), fValues);
   }  // end loop over cuts
}


//___________________________________________________________________________
void AliReducedAnalysisJpsi2ee::RunTrackSelection() {
   //
   // select electron candidates and prefilter tracks
   //
   // clear the track arrays
   fPosTracks.Clear("C"); fNegTracks.Clear("C"); fPrefilterPosTracks.Clear("C"); fPrefilterNegTracks.Clear("C");
   
   // loop over the track list and evaluate all the track cuts
   AliReducedTrackInfo* track = 0x0;
   TClonesArray* trackList = fEvent->GetTracks();
   TIter nextTrack(trackList);
   Float_t nsigma = 0.;
   for(Int_t it=0; it<fEvent->NTracks(); ++it) {
      track = (AliReducedTrackInfo*)nextTrack();
      AliReducedVarManager::FillTrackInfo(track, fValues);
      fHistosManager->FillHistClass("Track_BeforeCuts", fValues);
      for(UInt_t iflag=0; iflag<AliReducedVarManager::kNTrackingFlags; ++iflag) {
         AliReducedVarManager::FillTrackingFlag(track, iflag, fValues);
         fHistosManager->FillHistClass("TrackStatusFlags_BeforeCuts", fValues);
      }
      for(Int_t iLayer=0; iLayer<6; ++iLayer) {
         AliReducedVarManager::FillITSlayerFlag(track, iLayer, fValues);
         fHistosManager->FillHistClass("TrackITSclusterMap_BeforeCuts", fValues);
      }
      for(Int_t iLayer=0; iLayer<8; ++iLayer) {
         AliReducedVarManager::FillTPCclusterBitFlag(track, iLayer, fValues);
         fHistosManager->FillHistClass("TrackTPCclusterMap_BeforeCuts", fValues);
      }
      if(IsTrackSelected(track, fValues)) {
         if(track->Charge()>0) fPosTracks.Add(track);
         if(track->Charge()<0) fNegTracks.Add(track);
      }
      if(IsTrackPrefilterSelected(track, fValues)) {
         if(track->Charge()>0) fPrefilterPosTracks.Add(track);
         if(track->Charge()<0) fPrefilterNegTracks.Add(track);
      }
   }   // end loop over tracks
}


//___________________________________________________________________________
void AliReducedAnalysisJpsi2ee::RunSameEventPairing(TString pairClass /*="PairSE"*/) {
   //
   // Run the same event pairing
   //
   fValues[AliReducedVarManager::kNpairsSelected] = 0;
   
   TIter nextPosTrack(&fPosTracks);
   TIter nextNegTrack(&fNegTracks);
   
   AliReducedTrackInfo* pTrack=0;
   AliReducedTrackInfo* pTrack2=0;
   AliReducedTrackInfo* nTrack=0;
   AliReducedTrackInfo* nTrack2=0;
   for(Int_t ip=0; ip<fPosTracks.GetEntries(); ++ip) {
      pTrack = (AliReducedTrackInfo*)nextPosTrack();
      
      nextNegTrack.Reset();
      for(Int_t in=0; in<fNegTracks.GetEntries(); ++in) {
         nTrack = (AliReducedTrackInfo*)nextNegTrack();
         
         // verify that the two current tracks have at least 1 common bit
         if(!(pTrack->GetFlags() & nTrack->GetFlags())) continue;
         AliReducedVarManager::FillPairInfo(pTrack, nTrack, AliReducedPairInfo::kJpsiToEE, fValues);
         if(IsPairSelected(fValues)) {
            FillPairHistograms(pTrack->GetFlags() & nTrack->GetFlags(), 1, pairClass);    // 1 is for +- pairs 
            fValues[AliReducedVarManager::kNpairsSelected] += 1.0;
         }
      }  // end loop over negative tracks
      
      for(Int_t ip2=ip+1; ip2<fPosTracks.GetEntries(); ++ip2) {
         pTrack2 = (AliReducedTrackInfo*)fPosTracks.At(ip2);
         
         // verify that the two current tracks have at least 1 common bit
         if(!(pTrack->GetFlags() & pTrack2->GetFlags())) continue;
         AliReducedVarManager::FillPairInfo(pTrack, pTrack2, AliReducedPairInfo::kJpsiToEE, fValues);
         if(IsPairSelected(fValues)) {
            FillPairHistograms(pTrack->GetFlags() & pTrack2->GetFlags(), 0, pairClass);       // 0 is for ++ pairs 
            fValues[AliReducedVarManager::kNpairsSelected] += 1.0;
         }
      }  // end loop over positive tracks
   }  // end loop over positive tracks
   
   nextNegTrack.Reset();
   for(Int_t in=0; in<fNegTracks.GetEntries(); ++in) {
      nTrack = (AliReducedTrackInfo*)nextNegTrack();
      
      for(Int_t in2=in+1; in2<fNegTracks.GetEntries(); ++in2) {
         nTrack2 = (AliReducedTrackInfo*)fNegTracks.At(in2);
         
         // verify that the two current tracks have at least 1 common bit
         if(!(nTrack->GetFlags() & nTrack2->GetFlags())) continue;
         AliReducedVarManager::FillPairInfo(nTrack, nTrack2, AliReducedPairInfo::kJpsiToEE, fValues);
         if(IsPairSelected(fValues)) {
            FillPairHistograms(nTrack->GetFlags() & nTrack2->GetFlags(), 2, pairClass);      // 2 is for -- pairs
            fValues[AliReducedVarManager::kNpairsSelected] += 1.0;
         }
      }  // end loop over negative tracks
   }  // end loop over negative tracks
}


//___________________________________________________________________________
void AliReducedAnalysisJpsi2ee::RunPrefilter() {
   //
   // Run the prefilter selection
   // At this point it is assumed that the track lists are filled
   //
   TIter nextPosTrack(&fPosTracks);
   TIter nextNegTrack(&fNegTracks);
   TIter nextPosPrefilterTrack(&fPrefilterPosTracks);
   TIter nextNegPrefilterTrack(&fPrefilterNegTracks);
   
   // First pair the positive trackes with the prefilter selected tracks
   AliReducedTrackInfo* track=0;
   AliReducedTrackInfo* trackPref=0;
   for(Int_t ip = 0; ip<fPosTracks.GetEntries(); ++ip) {
      track = (AliReducedTrackInfo*)nextPosTrack();
      
      nextPosPrefilterTrack.Reset();
      for(Int_t ipp = 0; ipp<fPrefilterPosTracks.GetEntries(); ++ipp) {
         trackPref = (AliReducedTrackInfo*)nextPosPrefilterTrack();
         
         if(track->TrackId()==trackPref->TrackId()) continue;       // avoid self-pairing
         AliReducedVarManager::FillPairInfo(track, trackPref, AliReducedPairInfo::kJpsiToEE, fValues);
         if(!IsPairPreFilterSelected(fValues)) {
            track->ResetFlags(); 
            break;
         }
         else {
         }
      }  // end loop over positive prefilter tracks
      if(!track->GetFlags()) {
         fPosTracks.Remove(track); 
         continue;
      }
      
      nextNegPrefilterTrack.Reset();
      for(Int_t ipn = 0; ipn<fPrefilterNegTracks.GetEntries(); ++ipn) {
         trackPref = (AliReducedTrackInfo*)nextNegPrefilterTrack();
         
         AliReducedVarManager::FillPairInfo(track, trackPref, AliReducedPairInfo::kJpsiToEE, fValues);
         if(!IsPairPreFilterSelected(fValues)) {
            track->ResetFlags(); 
            break;
         }
         else {
         }
      }  // end loop over negative prefilter tracks
      
      if(!track->GetFlags()) {
         fPosTracks.Remove(track);
      }
   }  // end loop over the positive tracks

   for(Int_t in = 0; in<fNegTracks.GetEntries(); ++in) {
      track = (AliReducedTrackInfo*)nextNegTrack();
      
      nextPosPrefilterTrack.Reset();
      for(Int_t ipp = 0; ipp<fPrefilterPosTracks.GetEntries(); ++ipp) {
         trackPref = (AliReducedTrackInfo*)nextPosPrefilterTrack();
         
         AliReducedVarManager::FillPairInfo(track, trackPref, AliReducedPairInfo::kJpsiToEE, fValues);
         if(!IsPairPreFilterSelected(fValues)) {
            track->ResetFlags(); 
            break;
         }
      }  // end loop over positive prefilter tracks
      if(!track->GetFlags()) {
         fNegTracks.Remove(track); 
         continue;
      }
      
      nextNegPrefilterTrack.Reset();
      for(Int_t ipn = 0; ipn<fPrefilterNegTracks.GetEntries(); ++ipn) {
         trackPref = (AliReducedTrackInfo*)nextNegPrefilterTrack();
         
         if(track->TrackId()==trackPref->TrackId()) continue;       // avoid self-pairing
         AliReducedVarManager::FillPairInfo(track, trackPref, AliReducedPairInfo::kJpsiToEE, fValues);
         if(!IsPairPreFilterSelected(fValues)) {
            track->ResetFlags(); 
            break;
         }
      }  // end loop over negative prefilter tracks
      if(!track->GetFlags()) {
         fNegTracks.Remove(track);
      }
   }  // end loop over the negative tracks
}

//___________________________________________________________________________
void AliReducedAnalysisJpsi2ee::Finish() {
  //
  // run stuff after the event loop
  //
   if(fOptionRunMixing)
     fMixingHandler->RunLeftoverMixing(AliReducedPairInfo::kJpsiToEE);
}