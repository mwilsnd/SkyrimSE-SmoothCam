import gfx.io.GameDelegate;

class Crosshair3D extends MovieClip {
	public function Crosshair3D() {
		super();
		gotoAndStop(1);
	}
	
	function InitExtensions(): Void
	{
		GameDelegate.addCallBack("SetCrosshairPosition", this, "SetCrosshairPosition");
		GameDelegate.addCallBack("SetCrosshairSize", this, "SetCrosshairSize");
		GameDelegate.addCallBack("ShowCrosshair3D", this, "ShowCrosshair3D");
	}
	
	function ShowCrosshair3D(show: Boolean): Void
	{
		if (show) {
			_alpha = 100;
		} else {
			_alpha = 0;
		}
	}

	function SetCrosshairPosition(x: Number, y: Number): Void
	{
		_x = x;
		_y = y;
	}
	
	function SetCrosshairSize(w: Number, h: Number): Void
	{
		_width = w;
		_height = h;
	}
}
