Strict

'version 6
' - working on osx
'version 5
' - updated unsupported stub
'version 4
' - updated example
' - changed function name to ChangeScreenMode()
' - added GetAvailableScreenModes()
' - added SreenModeExists()
'version 3
' - made so example adds the monkey font
'version 2
' - first commit as proper module
'version 1
' - old version for monkey v67

Import mojo

#if TARGET <> "glfw"
	Function ChangeScreenMode:Void(width:Int, height:Int, depth:Int, fullscreen:Bool)
		' --- do nothing ---
		#IF CONFIG = "debug"
			Print "cant change screen mode on this target"
		#End
	End
	
	Function GetAvailableScreenModes:Int[][] ()
		' --- do nothing ---
		#IF CONFIG = "debug"
			Print "cant find available screen modes on this target"
		#End
		Return New Int[0][]
	End
	
	Function ScreenModeExists:Bool(width:Int, height:Int, depth:Int)
		' --- do nothing ---
		#IF CONFIG = "debug"
			Print "cant find available screen modes on this target"
		#End
		Return False
	End
	
	Function AddAutoLoadImage:Void(image:Image)
		' --- do nothing ---
	End
	
	Function RemoveAutoLoadImage:Void(image:Image)
		' --- do nothing ---
	End
	
	Function ClearAutoLoadImages:Void()
		' --- do nothing ---		
	End
#Else
	Import mojo.graphicsdevice
	
	Import "native/screenres.${TARGET}.${LANG}"
	
	Extern Private
		Function ChangeScreenModeNative:Void(width:Int, height:Int, depth:Int, fullscreen:Bool, surfaces:Surface[], surfacesTotal:Int)
	Private
	
	Extern
		Function GetAvailableScreenModes:Int[][] () = "GetAvailableScreenModesNative"
		Function ScreenModeExists:Bool(width:Int, height:Int, depth:Int) = "ScreenModeExistsNative"
	Public
		
	'private runtime stuff
	Private
	Global reloadImages:Image[]
	Global reloadImagesTotal:Int
	
	Class SurfaceMap<V> Extends Map<Surface, V>
		Method Compare:Int(a:Surface, b:Surface)
			If a = b Then Return 0
			Return -1
		End
	End
	Public
	
	'api
	Function ChangeScreenMode:Void(width:Int, height:Int, depth:Int, fullscreen:Bool)
		' --- changes the screen resolution ---
		'build a list of unique surfaces
		Local surfaceLookup:= New SurfaceMap<Surface>
		Local surface:Surface
		Local surfaceTotal:Int
		Local index:Int
		
		'iterate over images
		For index = 0 Until reloadImagesTotal
			'get surface
			surface = reloadImages[index].GetSurface()
			
			'already has it?
			If surfaceLookup.Contains(surface) = False
				'add to lookup
				surfaceLookup.Insert(surface, surface)
				surfaceTotal += 1
			EndIf
		Next
		
		'build surfaces array
		Local surfaces:Surface[surfaceTotal]
		index = 0
		
		For surface = EachIn surfaceLookup.Keys()
			surfaces[index] = surface
			index += 1
		Next
		
		'call change screen resolution
		ChangeScreenModeNative(width, height, depth, fullscreen, surfaces, surfaceTotal)
		
		'need to reset input device here as not possible without hacking in native
		
	End
	
	Function AddAutoLoadImage:Void(image:Image)
		' --- add another image to be reloaded when screen res is changed ---
		'check doesn't already have image
		If HasAutoLoadImage(image) Return
		
		'do we need to resize?
		If reloadImagesTotal >= reloadImages.Length reloadImages = reloadImages.Resize(reloadImages.Length * 2 + 10)
		
		'insert the image
		reloadImages[reloadImagesTotal] = image
		
		'increase count
		reloadImagesTotal += 1
	End
	
	Function RemoveAutoLoadImage:Void(image:Image)
		' --- remove an image from reload ---
		For Local index:= 0 Until reloadImagesTotal
			If reloadImages[index] = image
				'remove pointer to this image
				reloadImages[index] = Null
				
				'can we swap last image in to truncate array?
				'only if we are not currently scanning the last image
				If index < reloadImagesTotal - 1
					reloadImages[index] = reloadImages[reloadImagesTotal - 1]
					reloadImages[reloadImagesTotal - 1] = Null
				EndIf
				
				'decrease total
				reloadImagesTotal -= 1
				
				'done
				Return
			EndIf
		Next
	End
	
	Function ClearAutoLoadImages:Void()
		' --- clear all reload images ---
		'iterate and null
		For Local index:= 0 Until reloadImagesTotal
			reloadImages[index] = Null
		Next
		
		'reset total
		reloadImagesTotal = 0
	End
	
	Function HasAutoLoadImage:Bool(image:Image)
		' --- return true if has current reload image ---
		For Local index:= 0 Until reloadImagesTotal
			If reloadImages[index] = image Return True
		Next
		
		'nope
		Return False
	End
#End