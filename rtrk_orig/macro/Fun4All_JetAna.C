// headers
#include <G4_ActsGeom.C>
#include <G4_Global.C>
#include <G4_Magnet.C>
#include <GlobalVariables.C>
#include <Trkr_Clustering.C>
#include <Trkr_Reco.C>
#include <Trkr_RecoInit.C>
#include <Trkr_TpcReadoutInit.C>
#include <QA.C>
#include <Calo_Calib.C>  
#include <caloreco/CaloGeomMapping.h>  
#include <ffamodules/CDBInterface.h>
#include <globalvertex/GlobalVertexMap.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>
//fun4all
#include <fun4all/Fun4AllUtils.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
// ACTs Geometry
#include <trackreco/PHActsTrackProjection.h>
// my module
#include <rtrk/rtrk.h>
// jet reco macros
#include<NoBkgdSubJetReco.C>
#include<HIJetReco.C>

#include <fstream>
#include <vector>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libjetbase.so)
R__LOAD_LIBRARY(libjetbackground.so)
R__LOAD_LIBRARY(libcalo_reco.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(librtrk.so)
R__LOAD_LIBRARY(libg4centrality.so)
R__LOAD_LIBRARY(libg4dst.so)
R__LOAD_LIBRARY(libjetbase.so)
R__LOAD_LIBRARY(libg4jets.so)
R__LOAD_LIBRARY(libtrack_reco.so)

// Function to read a specific line from a file list
std::string getFileFromList(const std::string& listfile, int segment) {
  std::ifstream file(listfile);
  std::string line;
  int current_line = 0;
  
  while (std::getline(file, line)) {
    if (current_line == segment) {
      file.close();
      return line;
    }
    current_line++;
  }
  file.close();
  return "";
}

void Fun4All_JetAna(const int nEvents = -1,
                    const int segment = 0,
                    const std::string &filelistcalo = "dst_calo_cluster.list",
                    const std::string &filelistglobal = "dst_global.list",
                    const std::string &filelisttracks = "dst_tracks.list",	   
                    const std::string &filelisttruthjet = "dst_truth_jet.list",
                    const std::string &filelisttruth = "dst_truth.list",
                    const std::string &outname = "Jet_R_Tracks_MC")
{


  // Get specific files from lists based on segment
  std::string caloFile = getFileFromList(filelistcalo, segment);
  std::string globalFile = getFileFromList(filelistglobal, segment);
  std::string tracksFile = getFileFromList(filelisttracks, segment);
  std::string truthjetFile = getFileFromList(filelisttruthjet, segment);
  std::string truthFile = getFileFromList(filelisttruth, segment);

  // Check if all files were found
  if (caloFile.empty() || globalFile.empty() || tracksFile.empty() || 
      truthjetFile.empty() || truthFile.empty()) {
    std::cout << "ERROR: Could not find all files for segment " << segment << std::endl;
    std::cout << "Calo: " << caloFile << std::endl;
    std::cout << "Global: " << globalFile << std::endl;
    std::cout << "Tracks: " << tracksFile << std::endl;
    std::cout << "TruthJet: " << truthjetFile << std::endl;
    std::cout << "Truth: " << truthFile << std::endl;
    gSystem->Exit(1);
  }

  // Create output directory path
  std::string outputDir = "/sphenix/user/abrahma/rtrk/macro/output_mc/";

  // Create output filename with segment
  char segStr[10];
  sprintf(segStr, "%06d", segment);
  std::string outputFile = outputDir + outname + "_" + segStr + ".root";  

  auto se = Fun4AllServer::instance();
  se->Verbosity(0);
  auto rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", segment);
  Enable::CDB = true;
  rc->set_StringFlag("CDB_GLOBALTAG", "newcdbtag");
  rc->set_uint64Flag("TIMESTAMP", segment);
  std::string geofile = CDBInterface::instance()->getUrl("Tracking_Geometry");
  std::cout << "Geometry file from CDB: " << geofile << std::endl;
  Fun4AllRunNodeInputManager *ingeo = new Fun4AllRunNodeInputManager("GeoIn");
  ingeo->AddFile(geofile);
  se->registerInputManager(ingeo);
  
  // Initialize tracking subsystems
  std::cout << "Initializing tracking..." << std::endl;
  TpcReadoutInit(segment);
  TrackingInit();  // This sets up ACTS geometry internally
  
  auto projection = new PHActsTrackProjection("CaloProjection");
  se->registerSubsystem(projection);
  std::cout << "Registered PHActsTrackProjection" << std::endl;
 
  // Register input managers
  Fun4AllInputManager *in1 = new Fun4AllDstInputManager("DSTcalo");
  in1->AddFile(caloFile);
  se->registerInputManager(in1);

  Fun4AllInputManager *in2 = new Fun4AllDstInputManager("DSTglobal");
  in2->AddFile(globalFile);
  se->registerInputManager(in2);

  Fun4AllInputManager *in3 = new Fun4AllDstInputManager("DSTtruthjet");
  in3->AddFile(truthjetFile);
  se->registerInputManager(in3);

  Fun4AllInputManager *in4 = new Fun4AllDstInputManager("DSTtracks");
  in4->AddFile(tracksFile);
  se->registerInputManager(in4);

  Fun4AllInputManager *in5 = new Fun4AllDstInputManager("DSTtruth");
  in5->AddFile(truthFile);  
  se->registerInputManager(in5);

  Enable::NSJETS = true;
  Enable::NSJETS_TOWER = true;
  Enable::NSJETS_MC = true;
  //Enable::NSJETS_TRUTH = true;
  NoBkgdSubJetReco();
  
  // Register my module
  rtrk *myJetKC = new rtrk("AntiKt_Tower_r04", outputFile.c_str());  
  se->registerSubsystem(myJetKC);


  std::cout << "==============================================" << std::endl;
  std::cout << "Processing segment: " << segment << std::endl;
  std::cout << "Input files:" << std::endl;
  std::cout << "  Calo: " << caloFile << std::endl;
  std::cout << "  Global: " << globalFile << std::endl;
  std::cout << "  Tracks: " << tracksFile << std::endl;
  std::cout << "  Truth Jet: " << truthjetFile << std::endl;
  std::cout << "  Truth: " << truthFile << std::endl;
  std::cout << "Output file: " << outputFile << std::endl;
  std::cout << "==============================================" << std::endl;
  
  se->run(nEvents);
  se->End();

  std::cout << "==============================================" << std::endl;
  std::cout << "Completed processing segment: " << segment << std::endl;
  std::cout << "Output file: " << outputFile << std::endl;
  std::cout << "==============================================" << std::endl;
  gSystem->Exit(0);
}
