!----------------------------------------------------------------------
!	File:		pt_install_dialog.res
!	Description:	resource file for Pro/TOOLKIT Install Test
!
!            PTC             File
!   Date   Version   Author  Vers  Comment
!--------- -------  -------- ----- ------------------------------------
!06-Apr-06 L-01-06    VSA     $$1  Created 
!11-May-11 L-05-47    shajoshi $$2 Updated for Creo rebranding
!13-May-11 L-05-47    shajoshi $$3 Rolled back #2
!----------------------------------------------------------------------

(Dialog pt_install_dialog
    (Components
        (PushButton                     CloseBtn)
        (Label                          Message)
        (Label                          Title)
    )

    (Resources
        (CloseBtn.Label                 "OK")
        (CloseBtn.TopOffset             4)
        (CloseBtn.BottomOffset          4)
        (CloseBtn.LeftOffset            4)
        (CloseBtn.RightOffset           4)
        (Message.Label                  "Pro/TOOLKIT Installation Test Status")
        (Message.TopOffset              4)
        (Message.BottomOffset           4)
        (Message.LeftOffset             4)
        (Message.RightOffset            4)
        (Title.Bitmap                  "pt_ptclogo.gif")
        (Title.TopOffset                4)
        (Title.BottomOffset             4)
        (Title.LeftOffset               4)
        (Title.RightOffset              4)
        (.Label                         "Install Test Results")
        (.StartLocation                 5)
	(.DialogStyle                   2)
        (.Resizeable                    False)
        (.DefaultButton                 "CloseBtn")
        (.Layout
            (Grid (Rows 1 0 0) (Cols 1)
                Title
                Message
                CloseBtn
            )
        )
    )
)
