var samples_size;
var samples=[];

function draw() {
	var canvas = document.getElementById("canvas");
	if (null==canvas || !canvas.getContext) return;
	
	var MarginLeft = 0.1*canvas.width;
	var MarginRight = 0.2*canvas.width;
	var MarginTop = 0.1*canvas.height;
	var MarginBottom = 0.3*canvas.height;	

	var axes={}, ctx=canvas.getContext("2d");
	axes.xWidth = canvas.width-(MarginLeft+MarginRight); 
	axes.yHeight = canvas.height-(MarginTop+MarginBottom);	
	axes.x0 = MarginLeft;  // x0 pixels from left to x=0
	axes.y0 = MarginTop; // y0 pixels from top to y=0
	axes.doNegativeX = false;	
	//axes.scale = axes.y0;
		
	//var w = canvas.width;
	//var h = canvas.height; 	

	var SamplesToPlot={};
	SamplesToPlot.content = samples;
	//var max_of_array = Math.max.apply(Math, array);
	SamplesToPlot.maxvalue = Math.max.apply(Math, samples);
	SamplesToPlot.minvalue = Math.min.apply(Math, samples);
	//SamplesToPlot.maxvalue = 9;
	//SamplesToPlot.minvalue = 0;
	//SamplesToPlot.range = SamplesToPlot.maxvalue - SamplesToPlot.minvalue;
	
	// Calculating the position of the X-Axis on the Y-Axis
	if (SamplesToPlot.maxvalue < 0) {
		SamplesToPlot.maxvalue = 0;
		axes.XaxisPos = axes.y0;  
		SamplesToPlot.range = 0 - SamplesToPlot.minvalue;
		}
	else if (SamplesToPlot.minvalue > 0) {
			SamplesToPlot.minvalue = 0;
			axes.XaxisPos = axes.y0+axes.yHeight;
			SamplesToPlot.range = SamplesToPlot.maxvalue;
			}
		else	{
			SamplesToPlot.range = SamplesToPlot.maxvalue - SamplesToPlot.minvalue;			
			//SamplesToPlot.range = SamplesToPlot.maxvalue - ;			
			axes.XaxisPos = axes.y0 + ((SamplesToPlot.maxvalue/SamplesToPlot.range)*axes.yHeight);		 
			}
	
	showAxes(ctx,axes);
	//funGraph(ctx,axes,fun1,"rgb(11,153,11)",1); 
	funGraph(ctx,axes,SamplesToPlot,"rgb(66,44,255)",2);
}

function funGraph (ctx,axes,SamplesToPlot,color,thick) {
	var xx, yy, dx, x0=axes.x0, y0=axes.y0, scale=axes.scale;
	//var iMax = Math.round((ctx.canvas.width)/dx);
	//var iMin = axes.doNegativeX ? Math.round(-x0/dx) : 0;
	ctx.beginPath();
	ctx.lineWidth = thick;
	ctx.strokeStyle = color;

	dx = axes.xWidth / (samples_size-1);

	for (var i=0;i<samples_size;i++) {
		xx = x0+dx*i; 
		yy = samples[i];
		if (i==0) ctx.moveTo(xx,axes.y0+(((SamplesToPlot.maxvalue-yy)/SamplesToPlot.range)*axes.yHeight));
		//first sample = -10...range= 140, minvalue=-120.... maxvalue=20    (20--10)/140
	else         ctx.lineTo(xx,axes.y0+(((SamplesToPlot.maxvalue-yy)/SamplesToPlot.range)*axes.yHeight));
	}
	ctx.stroke();
	ctx.fillStyle = "black";
	ctx.font = "bold 13px sans-serif";
	ctx.fillText("NUMBER OF SAMPLES = ", 20,y0+axes.yHeight+20);
	ctx.fillText(String(samples_size), 20+200,y0+axes.yHeight+20);
	//ctx.fillText("X-axis = SAMPLE#", axes.x0+axes.xWidth+5,y0);	
	ctx.fillText("SAMPLE Max VALUE = ", 20,y0+axes.yHeight+40);
	ctx.fillText(String(SamplesToPlot.maxvalue), 20+200,y0+axes.yHeight+40);	
	ctx.fillText("SAMPLE Min VALUE = ", 20,y0+axes.yHeight+60);	
	ctx.fillText(String(SamplesToPlot.minvalue), 20+200,y0+axes.yHeight+60);	
	}


function showAxes(ctx,axes) {
	var x0=axes.x0, 	w=ctx.canvas.width;
	var y0=axes.XaxisPos, 	h=ctx.canvas.height;
	
	//var xmin = axes.doNegativeX ? 0 : x0;
	ctx.beginPath();
	ctx.strokeStyle = "rgb(0,0,0)"; 
	ctx.moveTo(x0,y0); ctx.lineTo(x0+axes.xWidth,y0);  // X axis
	ctx.moveTo(x0,axes.y0);    ctx.lineTo(x0,axes.y0+axes.yHeight);  // Y axis
	ctx.stroke();
	ctx.fillStyle = "black";
	ctx.font = "bold 13px sans-serif";
	// Now use green paint
	//ctx.fill();
	// And fill area under curve
	ctx.fillText("Y-axis = SAMPLE AMPLITUDE", 20,10);
	ctx.fillText("X-axis = SAMPLE#", axes.x0+axes.xWidth+5,y0);
}
