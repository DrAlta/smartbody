/*
** ADOBE SYSTEMS INCORPORATED
** Copyright 2012 Adobe Systems Incorporated
** All Rights Reserved.
**
** NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the
** terms of the Adobe license agreement accompanying it.  If you have received this file from a
** source other than Adobe, then your use, modification, or distribution of it requires the prior
** written permission of Adobe.
*/
package com.adobe.flascc
{
  import flash.display.Bitmap;
  import flash.display.BitmapData;
  import flash.display.DisplayObjectContainer;
  import flash.display.Loader;
  import flash.display.Sprite;
  import flash.display.Stage3D;
  import flash.display.StageAlign;
  import flash.display.StageScaleMode;
  import flash.display3D.Context3D;
  import flash.display3D.Context3DRenderMode;
  import flash.events.AsyncErrorEvent;
  import flash.events.Event;
  import flash.events.EventDispatcher;
  import flash.events.IOErrorEvent;
  import flash.events.KeyboardEvent;
  import flash.events.MouseEvent;
  import flash.events.ProgressEvent;
  import flash.events.SampleDataEvent;
  import flash.events.SecurityErrorEvent;
  import flash.geom.Rectangle;
  import flash.media.Sound;
  import flash.media.SoundChannel;
  import flash.net.LocalConnection;
  import flash.net.URLLoader;
  import flash.net.URLLoaderDataFormat;
  import flash.net.URLRequest;
  import flash.text.TextField;
  import flash.ui.Mouse;
  import flash.utils.ByteArray
  import flash.utils.ByteArray;
  import flash.utils.Endian;
  import flash.utils.getTimer;

  import GLS3D.GLAPI; 
  import com.adobe.flascc.CModule;
  import com.adobe.flascc.vfs.HTTPBackingStore;
  import com.adobe.flascc.vfs.ISpecialFile;
  import com.adobe.flascc.vfs.LSOBackingStore;
  import com.adobe.flascc.vfs.RootFSBackingStore;
  import com.adobe.flascc.vfs.InMemoryBackingStore;
  import com.adobe.flascc.vfs.zip.*;

  public class ZipBackingStore extends InMemoryBackingStore {
    public function ZipBackingStore()
    {
      addDirectory("/root")
      addDirectory("/root/data")
    }

    public function addZip(data:ByteArray) {
      var zip = new ZipFile(data)
      for (var i = 0; i < zip.entries.length; i++) {
        var e = zip.entries[i]
        if (e.isDirectory()) {
          addDirectory("/root/data/"+e.name)
        } else {
          addFile("/root/data/"+e.name, zip.getInput(e))
        }
      }
    }
  }  
  
  var zfs:ZipBackingStore = new ZipBackingStore();
  public function addVFSZip(x:*) {
    if(!zfs) {
      zfs = new ZipBackingStore();
    }
    zfs.addZip(x)
  }   
  
  
  /**
  * A basic implementation of a console for FlasCC apps.
  * The PlayerKernel class delegates to this for things like read/write
  * so that console output can be displayed in a TextField on the Stage.
  */
  public class Console extends Sprite implements ISpecialFile
  {
    private var enableConsole:Boolean = true
	private var enableDrawing:Boolean = true
	private static var _width:int = 800;
    private static var _height:int = 600;
    private var _tf:TextField
    private var inputContainer:DisplayObjectContainer

    private var _stage:Stage3D;
    private var _context:Context3D;
    private var rendered:Boolean = false;
    private var inited:Boolean = false
    private const emptyVec:Vector.<int> = new Vector.<int>()
	private var mainloopTickPtr:int

    /**
    * To Support the preloader case you might want to have the Console
    * act as a child of some other DisplayObjectContainer.
    */
    public function Console(container:DisplayObjectContainer = null)
    {
      CModule.rootSprite = container ? container.root : this

      if(CModule.runningAsWorker()) {
        return;
      }

      if(container) {
        container.addChild(this)
        init(null)
      } else {
        addEventListener(Event.ADDED_TO_STAGE, init)
      }
    }

    /**
    * All of the real FlasCC init happens in this method
    * which is either run on startup or once the SWF has
    * been added to the stage.
    */
    protected function init(e:Event):void
    {
      inputContainer = new Sprite()
      addChild(inputContainer)

      addEventListener(Event.ENTER_FRAME, enterFrame)
	  stage.align = StageAlign.TOP_LEFT;
      stage.scaleMode = StageScaleMode.NO_SCALE;
      stage.frameRate = 60
      stage.scaleMode = StageScaleMode.NO_SCALE

      if(enableConsole) {
        _tf = new TextField
        _tf.multiline = true
        _tf.width = stage.stageWidth
        _tf.height = stage.stageHeight 
        inputContainer.addChild(_tf)
      }
	  
	  if (enableDrawing)
	  {
		_stage = stage.stage3Ds[0];
		_stage.addEventListener(Event.CONTEXT3D_CREATE, context_created);
		//_stage.requestContext3D(Context3DRenderMode.AUTO);
		_stage.requestContext3D("auto");	  
	  }
    }
	
    private function context_created(e:Event):void
   {
      _context = _stage.context3D;
      _context.configureBackBuffer(_width, _height, 4, true /*enableDepthAndStencil*/ );
      _context.enableErrorChecking = false;
      
      trace(_context.driverInfo);
      GLAPI.init(_context, null, stage);
      var gl:GLAPI = GLAPI.instance;
      gl.context.clear(0.8, 0.8, 0.8);
      gl.context.present();
      gl.context.clear(0.8, 0.8, 0.8);     
      stage.addEventListener(Event.RESIZE, stageResize);
    }
	
    private function stageResize(event:Event):void
    {
        // need to reconfigure back buffer
        _width = stage.stageWidth;
        _height = stage.stageHeight;
        _context.configureBackBuffer(_width, _height, 4, true /*enableDepthAndStencil*/ );
    }	

    /**
    * The callback to call when FlasCC code calls the posix exit() function. Leave null to exit silently.
    * @private
    */
    public var exitHook:Function;

    /**
    * The PlayerKernel implementation will use this function to handle
    * C process exit requests
    */
    public function exit(code:int):Boolean
    {
      // default to unhandled
      return exitHook ? exitHook(code) : false;
    }

    /**
    * The PlayerKernel implementation will use this function to handle
    * C IO write requests to the file "/dev/tty" (e.g. output from
    * printf will pass through this function). See the ISpecialFile
    * documentation for more information about the arguments and return value.
    */
    public function write(fd:int, bufPtr:int, nbyte:int, errnoPtr:int):int
    {
      var str:String = CModule.readString(bufPtr, nbyte)
      consoleWrite(str)
      return nbyte
    }

    /**
    * The PlayerKernel implementation will use this function to handle
    * C IO read requests to the file "/dev/tty" (e.g. reads from stdin
    * will expect this function to provide the data). See the ISpecialFile
    * documentation for more information about the arguments and return value.
    */
    public function read(fd:int, bufPtr:int, nbyte:int, errnoPtr:int):int
    {
      return 0
    }

    /**
    * The PlayerKernel implementation will use this function to handle
    * C fcntl requests to the file "/dev/tty" 
    * See the ISpecialFile documentation for more information about the
    * arguments and return value.
    */
    public function fcntl(fd:int, com:int, data:int, errnoPtr:int):int
    {
      return 0
    }

    /**
    * The PlayerKernel implementation will use this function to handle
    * C ioctl requests to the file "/dev/tty" 
    * See the ISpecialFile documentation for more information about the
    * arguments and return value.
    */
    public function ioctl(fd:int, com:int, data:int, errnoPtr:int):int
    {
      return 0
    }

    /**
    * Helper function that traces to the flashlog text file and also
    * displays output in the on-screen textfield console.
    */
    protected function consoleWrite(s:String):void
    {
      trace(s)
      if(enableConsole) {
        _tf.appendText(s)
        _tf.scrollV = _tf.maxScrollV
      }
    }

    /**
    * The enterFrame callback will be run once every frame. UI thunk requests should be handled
    * here by calling CModule.serviceUIRequests() (see CModule ASdocs for more information on the UI thunking functionality).
    */
    protected function enterFrame(e:Event):void
    {
		if(!inited) 
		{
			inited = true
			CModule.vfs.console = this
			CModule.vfs.addBackingStore(zfs, null)
			
			// for some weird reasons, addBackingStore didn't mount the folder correctly, i still need to explicitly mount directories
			var paths = zfs.getPaths()
			for (var i = 0; i < paths.length; i++)
			{
				var path:String = paths[i]
				if (zfs.isDirectory(path))
				{
					var status = CModule.vfs.checkPath(path)
					if (path == "/" || path == "/root" || path == "/root/data")
						continue;
					trace("path added: " + path + " " + status)
					CModule.vfs.addDirectory(path)
				}
			}			
			
			
			CModule.startAsync(this, null)
	        mainloopTickPtr = CModule.getPublicSymbol("mainLoop")
		}
		CModule.serviceUIRequests()
		CModule.callI(mainloopTickPtr, emptyVec);
		
		if (enableDrawing)
		{
			var gl:GLAPI = GLAPI.instance;
			gl.context.present();
			gl.context.clear(0.8, 0.8, 0.8);
		}
    }

    /**
    * Provide a way to get the TextField's text.
    */
    public function get consoleText():String
    {
        var txt:String = null;

        if(_tf != null){
            txt = _tf.text;
        }
        
        return txt;
    }
  }
}
