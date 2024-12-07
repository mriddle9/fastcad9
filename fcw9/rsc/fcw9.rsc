XPID 0x7000 // fcw9 main program Resources

//	text strings

*1,SimpleApp 
//	User name for about dialog
*2,Mike 

//	images

IMAGE 200,"linen.png"
IMAGE 201,"AboutDlg.png"
IMAGE 202,"Arabic.png"

// dialogs

MLTEXT 100
DIALOG,800,490,230,"About FastCAD v9",MODAL
{
IMAGE,500,PARENT,40,34,64,64,"AboutDlg.png",ROUNDED
LABEL,510,PARENT,150,10,-10,24,"Program (c)2012-2024 Evolution Computing Inc.",NONE
LABEL,511,PARENT,150,34,-10,24,"All rights reserved",NONE
LABEL,512,PARENT,150,58,-10,24,"Version: 0.00",NONE
LABEL,513,PARENT,150,82,-10,24,"Date: 0.00",NONE
LABEL,-1,PARENT,150,108,119,24,"Registered to:",NONE
LABEL,514,PARENT,250,108,-10,24,"",NONE
PUSHBUTTON,598,PARENT,196,150,80,24,"Ok",ENDDLG
}
END
*

END
