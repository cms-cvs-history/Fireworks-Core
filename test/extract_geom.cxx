{
   // gSystem->Load("libFireworksCore");
   DetIdToMatrix map;
   const char* geomtryFile = "cmsGeom10.root";
   map.loadGeometry( geomtryFile );
   map.loadMap( geomtryFile );

   // display the extract
   TEveManager::Create();
   /*
   TEveGeoShapeExtract* container = new TEveGeoShapeExtract( "MuonRhoZ" );
              container->AddElement( map.getExtract( 588349440 ) );
              container->AddElement( map.getExtract( 584155136 ) );
              container->AddElement( map.getExtract( 579960832 ) );
              container->AddElement( map.getExtract( 575766528 ) );
   
              container->AddElement( map.getExtract( 577339392 ) );
              container->AddElement( map.getExtract( 581533696 ) );
              container->AddElement( map.getExtract( 585728000 ) );
              container->AddElement( map.getExtract( 590970880 ) );
   TFile f("muonz.root", "RECREATE");
   container->Write("Extract");
   f.Close();
   TEveGeoShape::ImportShapeExtract( container, 0);
    */
   TEveGeoShape::ImportShapeExtract(map.getAllExtracts(),0);
   /*
   TEveGeoShape::ImportShapeExtract( map.getExtract( 575766528 ),0 );
   TEveGeoShape::ImportShapeExtract( map.getExtract( 584155136 ),0 );
   TEveGeoShape::ImportShapeExtract( map.getExtract( 579960832 ),0 );
   TEveGeoShape::ImportShapeExtract( map.getExtract( 575766528 ),0 );
   
   TEveGeoShape::ImportShapeExtract( map.getExtract( 577339392 ),0 );
   TEveGeoShape::ImportShapeExtract( map.getExtract( 581533696 ),0 );
   TEveGeoShape::ImportShapeExtract( map.getExtract( 585728000 ),0 );
   TEveGeoShape::ImportShapeExtract( map.getExtract( 590970880 ),0 );
   
   for ( Int_t i=0; i<1000; ++i) {
      TEveGeoShapeExtract* extract = map.getExtract(574980096+(i << 18));
      if ( extract ) TEveGeoShape::ImportShapeExtract(extract,0);
   }
   */
   // TEveGeoShape* extract = TEveGeoShape::ImportShapeExtract(map.getExtract(575176704),0);
   
   //   TEveElementList* eveTopElement = new TEveElementList("CMS");
   //   gEve->AddGlobalElement( eveTopElement );
   // TEveGeoTopNode* eveTopNode = new TEveGeoTopNode(gGeoManager, extract);
   //    // eveTopNode->UseNodeTrans();
   //    // gEve->AddGlobalElement(eveTopNode, eveTopNodeElement);
   // gEve->AddGlobalElement(eveTopElement);
}

