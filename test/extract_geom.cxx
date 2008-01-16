{
   gSystem->Load("libFireworksCore");
   DetIdToMatrix map;
   const char* geomtryFile = "cmsGeom10.root";
   map.loadGeometry( geomtryFile );
   map.loadMap( geomtryFile );

   // display the extract
   TEveManager::Create();
   TEveGeoShape::ImportShapeExtract(map.getAllExtracts(),0);
   
   // for ( Int_t i=0; i<10000; ++i) {
   // TEveGeoShapeExtract* extract = map.getExtract(0x22408000+(i << 18));
   // if ( extract ) TEveGeoShape::ImportShapeExtract(extract,0);
   //}
   
   // TEveGeoShape* extract = TEveGeoShape::ImportShapeExtract(map.getExtract(575176704),0);
   
   //   TEveElementList* eveTopElement = new TEveElementList("CMS");
   //   gEve->AddGlobalElement( eveTopElement );
   // TEveGeoTopNode* eveTopNode = new TEveGeoTopNode(gGeoManager, extract);
   //    // eveTopNode->UseNodeTrans();
   //    // gEve->AddGlobalElement(eveTopNode, eveTopNodeElement);
   // gEve->AddGlobalElement(eveTopElement);
}

