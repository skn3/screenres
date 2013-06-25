//forward declaration
extern gxtkGraphics *bb_graphics_device;

static void ChangeScreenResolutionNative(int width, int height, int depth, bool fullscreen,Array<gxtkSurface* > surfaces,int surfacesTotal) {
	// --- change the screen resolution ---
	//this function will also handle transfering surfaces to the nw context.
	int index;
	gxtkSurface* surface;

	//get details about the change
	//work out the bits
	int redBits,greenBits,blueBits,alphaBits,stencilBits;
	switch(depth) {
		case 0:
			//use desktop settings
			fullscreen = false;
			redBits = 0;
			greenBits = 0;
			blueBits = 0;
			alphaBits = 0;
			break;
		case 16:
			//16bit mode we give priority to green bit
			redBits = 5;
			greenBits = 6;
			blueBits = 5;
			alphaBits = 0;
			break;
		case 24:
		case 32:
			//fix 24bits for potential later cross compatability (faking alpha bits)
			redBits = 8;
			greenBits = 8;
			blueBits = 8;
			alphaBits = 0;
			break;
	}

	//get desktop mode
	GLFWvidmode desktopMode;
	glfwGetDesktopMode( &desktopMode );
	
	//get new width/height for window
	if( !width ) width=desktopMode.Width;
	if( !height ) height=desktopMode.Height;
	
	//get position for window
	int x,y;
	if (fullscreen == false) {
		//windowed mode
		x = (desktopMode.Width-width)/2;
		y = (desktopMode.Height-height)/2;
	} else {
		//desktop mode
		x = 0;
		y = 0;
	}
	
	//setup window hint
	glfwOpenWindowHint( GLFW_WINDOW_NO_RESIZE,CFG_GLFW_WINDOW_RESIZABLE ? GL_FALSE : GL_TRUE );
	
	//simple or complex window recreation?
	if (surfacesTotal == 0) {
		//none to transfer
		//close the previous window
		glfwCloseWindow();
		
		//create new window
		if(!glfwOpenWindow(width,height,redBits,greenBits,blueBits,alphaBits,CFG_OPENGL_DEPTH_BUFFER_ENABLED ? 32 : 0,stencilBits,fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW)) {
			puts( "glfwOpenWindow failed" );
			exit(-1);
		}
	} else {
		//yup need to transfer surfaces
		//before closing the old window we need to download all surfaces that are marked
		glEnable(GL_TEXTURE_2D);
		
		//create vector to store data pointers
		std::vector<GLubyte *> surfacesData(surfacesTotal);
		std::vector<int> surfacesFormat(surfacesTotal);
		
		//iterate over surfaces and transfer the data
		for (index = 0; index < surfacesTotal;++index) {
			//get surface
			surface = surfaces[index];

			//bind the texture
			glBindTexture(GL_TEXTURE_2D,surface->texture);
						
			//create data in system memory
			surfacesData[index] = (GLubyte *)malloc(surface->width*surface->height*4);
			
			//get the format
			switch( surface->depth ){
				case 1:
					surfacesFormat[index] = GL_LUMINANCE;
					break;
				case 2:
					surfacesFormat[index] = GL_LUMINANCE_ALPHA;
					break;
				case 3:
					surfacesFormat[index] = GL_RGB;
					break;
				case 4:
					surfacesFormat[index] = GL_RGBA;
					break;
			}
			
			//transfer from video memory to system memory
			glGetTexImage(GL_TEXTURE_2D, 0, surfacesFormat[index], GL_UNSIGNED_BYTE, surfacesData[index]);
			
			//unbind it
			glBindTexture(GL_TEXTURE_2D,0);
		}
	
		//close the previous window
		glfwCloseWindow();
		
		//create new window
		if(!glfwOpenWindow(width,height,redBits,greenBits,blueBits,alphaBits,CFG_OPENGL_DEPTH_BUFFER_ENABLED ? 32 : 0,stencilBits,fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW)) {
			puts( "glfwOpenWindow failed" );
			exit(-1);
		}
		
		//transfer surface data back into video memory
		bool ok;
		int texwidth,texheight;
		
		//iterate over all surfaces
		for (index = 0; index < surfacesTotal;++index) {
			//get surface
			surface = surfaces[index];
			
			glGenTextures( 1,&surface->texture );
			glBindTexture( GL_TEXTURE_2D,surface->texture );
		
			if( CFG_MOJO_IMAGE_FILTERING_ENABLED ){
				glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
			}else{
				glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
				glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST );
			}
		
			glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE );
		
			ok = true;
			
			texwidth = surface->width;
			texheight = surface->height;
			
			glTexImage2D( GL_TEXTURE_2D,0,surfacesFormat[index],texwidth,texheight,0,surfacesFormat[index],GL_UNSIGNED_BYTE,0);
			if( glGetError()!=GL_NO_ERROR ) {
				texwidth=Pow2Size(width);
				texheight=Pow2Size(height);
	
				glTexImage2D( GL_TEXTURE_2D,0,surfacesFormat[index],texwidth,texheight,0,surfacesFormat[index],GL_UNSIGNED_BYTE,0 );
				if( glGetError()!=GL_NO_ERROR ) ok=false;
			}
			
			if(ok){
				glPixelStorei( GL_UNPACK_ALIGNMENT,1 );
				glTexSubImage2D( GL_TEXTURE_2D,0,0,0,surface->width,surface->height,surfacesFormat[index],GL_UNSIGNED_BYTE,surfacesData[index]);
				
				surface->uscale = 1.0 / texwidth;
				surface->vscale = 1.0 / texheight;
			}
			
			//free the data in system memory
			free(surfacesData[index]);
		}
		
		//free memory used by vectors
		std::vector<GLubyte *>().swap(surfacesData);
		std::vector<int>().swap(surfacesFormat);
	}
		
	//update glfw stuff
	glfwSetWindowPos(x,y);	
	glfwSetWindowTitle(_STRINGIZE(CFG_GLFW_WINDOW_TITLE));
	glfwEnable( GLFW_KEY_REPEAT );
	glfwDisable( GLFW_AUTO_POLL_EVENTS );
	glfwSetKeyCallback( BBGlfwGame::OnKey );
	glfwSetCharCallback( BBGlfwGame::OnChar );
	glfwSetMouseButtonCallback( BBGlfwGame::OnMouseButton );
	glfwSetMousePosCallback( BBGlfwGame::OnMousePos );
	glfwSetWindowCloseCallback(	BBGlfwGame::OnWindowClose );
	
	//update monkey game stuff
	bb_graphics_device->width = width;
	bb_graphics_device->height = height;
}