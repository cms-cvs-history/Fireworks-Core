#ifndef Fireworks_Core_FWListViewObjectEditor_h
#define Fireworks_Core_FWListViewObjectEditor_h
// -*- C++ -*-
//
// Package:     Core
// Class  :     FWListViewObjectEditor
// 
/**\class FWListViewObjectEditor FWListViewObjectEditor.h Fireworks/Core/interface/FWListViewObjectEditor.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Mon Mar 10 09:02:56 CDT 2008
// $Id$
//

// system include files
#include <vector>
#include "TGedFrame.h"

// user include files

// forward declarations
class TGVerticalFrame;
class FWParameterSetterBase;

class FWListViewObjectEditor : public TGedFrame
{

   public:
      FWListViewObjectEditor(const TGWindow* p=0, Int_t width=170, Int_t height=30,
                             UInt_t options=kChildFrame, Pixel_t back=GetDefaultFrameBackground());
      virtual ~FWListViewObjectEditor();

      // ---------- const member functions ---------------------

      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------
      virtual void SetModel(TObject* obj);
      ClassDef(FWListViewObjectEditor, 0);

   private:
      FWListViewObjectEditor(const FWListViewObjectEditor&); // stop default

      const FWListViewObjectEditor& operator=(const FWListViewObjectEditor&); // stop default

      // ---------- member data --------------------------------
      TGVerticalFrame* m_frame;
      //can't use boost::shared_ptr because CINT will see this
      std::vector<FWParameterSetterBase*> m_setters;

};


#endif
