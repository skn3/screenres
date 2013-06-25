Strict

Import mojo
Import screenres

Function Main:Int()
	New MyApp
	Return 0
End

Class MyApp Extends App
	Field image1:Image
	Field image2:Image
	Field image3:Image
	
	Field large:= True
	
	Method OnCreate:Int()
		' --- setup the game ---
		SetUpdateRate(60)
		
		'load an image
		image1 = LoadImage("test.png")
		image2 = image1.GrabImage(2, 2, 12, 12)
		image3 = LoadImage("test2.png")
		
		'add image to reload
		AddAutoLoadImage(image1)
		AddAutoLoadImage(image2)'this wont do anything as it is a duplicate surface Grabbed from image1
		AddAutoLoadImage(image3)
		
		Local blah:Surface[10]
		blah[0] = Null
		
		Return 0
	End
	
	Method OnUpdate:Int()
		' --- update the game ---
		If KeyHit(KEY_ESCAPE) Error("")
		
		If KeyHit(KEY_SPACE)
			If large
				large = False
				ChangeScreenResolution(300, 300, 32, False)
			Else
				large = True
				ChangeScreenResolution(640, 480, 32, True)
			EndIf
		EndIf
		
		Return 0
	End
	
	Method OnRender:Int()
		' --- render the game ---
		Cls(180, 180, 180)
		
		'draw a rect to see if device width is correct
		SetColor(150, 150, 150)
		DrawRect(20, 20, DeviceWidth() -40, DeviceHeight() -40)
		
		'draw the images
		SetColor(255, 255, 255)
		If image1 DrawImage(image1, 50, 50, 0.0, 3.0, 3.0, 0)
		If image2 DrawImage(image2, 100, 100, 0.0, 3.0, 3.0, 0)
		If image3 DrawImage(image3, 150, 150, 0.0, 3.0, 3.0, 0)
		Return 0
	End
End