//
// theme.js
// Copyright (C) 2003-2006 MGTEK. All rights reserved.
//
// The contents of this file are confidential and proprietary
// to MGTEK and shall not be duplicated or used for any other
// purpose other than generating help files in conjunction with
// MGTEK Help Producer.
//

window.onload = OnLoad;
window.onresize = OnResize;
document.onmousedown = OnMouseDown;
document.onkeypress = OnKeyPress;

var g_Browser = top.g_Browser ? top.g_Browser : new Browser();
var g_NonScrollingHeader = top.g_NonScrollingHeader ? top.g_NonScrollingHeader : new NonScrollingHeader();
var g_Popup = top.g_Popup ? top.g_Popup : new Popup();

/////////////////////////////////////////////////////////////////////////////

function OnLoad()
{
	g_NonScrollingHeader.Enable();
	g_Popup.Enable();
}

function OnResize()
{
	g_NonScrollingHeader.Resize();
	g_Popup.Hide();
}

function OnMouseDown()
{
	g_Popup.Hide();
}

function OnKeyPress()
{
	g_Popup.Hide();
}

/////////////////////////////////////////////////////////////////////////////

function Browser()
{
	this.IsIE = false;
	this.IsIE4 = false;
	this.IsMacIE = false;
	this.IsSafari = false;

	this.IE = 0;
	this.Gecko = 0;
	this.Opera = 0;
	this.Safari = 0;
	this.ICab = 0;

	var agent = navigator.userAgent.toLowerCase();
	if (agent.indexOf("icab") >= 0)
	{
		// Mozilla/4.5 (compatible; iCab 2.9.8; Macintosh; U; PPC; Mac OS X)
		var pos = agent.indexOf("icab ");
		if (pos > 0)
			this.ICab = parseFloat(agent.substr(pos + 5));
	}
	else if (agent.indexOf("safari") >= 0)
	{
		// Mozilla/5.0 (Macintosh; U; PPC Mac OS X; en-us) AppleWebKit/85.7 (KHTML, like Gecko) Safari/85.5
		// 85.5 = V1.0
		// 85.8 = V1.0.3
		this.IsSafari = true;
		var pos = agent.indexOf("safari/");
		if (pos > 0)
			this.Safari = parseFloat(agent.substr(pos + 7));
	}
	else if (agent.indexOf("konqueror") >= 0)
	{
		// Broken browser.
		// Legacy mode (no script) only.
	}
	else if (agent.indexOf("opera") >= 0)
	{
		var pos = agent.indexOf("opera ");
		if (pos > 0)
			this.Opera = parseFloat(agent.substr(pos + 6));
	}
	else if (agent.indexOf("gecko") >= 0)
	{
	 	this.Gecko = 6;
	}
	else if (navigator.appName == "Microsoft Internet Explorer")
	{
		this.IsIE = true;
		this.IsMacIE = agent.indexOf("mac") >= 0;

		var pos = agent.indexOf("msie ");
		if (pos > 0)
		{
			this.IE = parseFloat(agent.substring(pos + 5, pos + 8));
			if (this.IE < 5)
				this.IsIE4 = true;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// Non-scrolling header

function NonScrollingHeader()
{
	this.Enable = function()
	{
		var mainSection = GetElementById(document, "mainSection");
		if (!mainSection)
			return;

		GetDocumentBody(document).style.overflow = "hidden";
		mainSection.style.overflow = "auto";

		this.Resize();
	}

	this.Resize = function()
	{
		var header = GetElementById(document, "header");
		var mainSection = GetElementById(document, "mainSection");
		if (!header || !mainSection)
			return;

		var body = GetDocumentBody(document);
		mainSection.style.width = body.clientWidth;
		mainSection.style.height = body.clientHeight - header.offsetHeight;
	}
}

/////////////////////////////////////////////////////////////////////////////
// HTML Popups

var L_ViewDefinition = "View Definition";

function Popup()
{
	this.Container = null;
	this.PopupDiv = null;
	this.PopupWindow = null;

	this.Enable = function()
	{
		var script = document.createElement("SCRIPT");
		script.language = "JavaScript";
		script.htmlFor = "popup";
		script.event = "onclick";
		script.text = "g_Popup.Create(this)";
		document.body.appendChild(script);

		for (var i = 0; i < document.anchors.length; i++)
		{
			var a = document.anchors[i];
			if (a.id.toLowerCase() == "popup")
			{
				a.title = L_ViewDefinition;
			}
		}
	}

	this.Create = function()
	{
		var source = window.event.srcElement;
		if (!source)
			return;

		window.event.returnValue = false;

		if (!this.Container)
		{
			if (g_Browser.IsIE4)
			{
				var html = "<IFRAME id='HpPopupContainer' style='visibility:hidden;position:absolute' frameborder='no'></IFRAME>";
				document.body.insertAdjacentHTML("beforeEnd", html);
				this.Container = document.all.HpPopupContainer;
			}
			else
			{
				this.Container = document.createElement("IFRAME");
				this.Container.id = "HpPopupContainer";
				this.Container.style.visibility = "hidden";
				this.Container.style.position = "absolute";
				this.Container.frameBorder = "no";
				document.body.appendChild(this.Container);
			}
		}
		else
		{
			this.Container.style.visibility = "hidden";
		}

		this.Container.src = source.href;
		this.Container.style.left = window.event.clientX;
		this.Container.style.top = window.event.clientY;
	}

	this.Hide = function()
	{
		if (this.PopupDiv)
			this.PopupDiv.style.visibility = "hidden";
		if (this.PopupWindow)
			this.PopupWindow.hide();
		this.PopupWindow = null;
	}

	this.OnLoad = function(doc)
	{
		var container = this.Container;
		if (!container)
			return;

		var x = container.style.pixelLeft;
		var y = container.style.pixelTop;
		var width = document.body.clientWidth * 50 / 100;

		if (window.createPopup)
		{
			this.PopupWindow = window.createPopup();
			for (var i = 0; i < doc.styleSheets.length; i++)
				this.PopupWindow.document.createStyleSheet(doc.styleSheets[i].href);
			this.PopupWindow.document.body.innerHTML = doc.body.innerHTML;
			var height = GetElementById(this.PopupWindow.document, "popup").scrollHeight;
			this.PopupWindow.show(x, y, width, 200, document.body);
		}
		else
		{
			container.style.visibility = "visible";
		}


/*
		var popupBody = GetElementById(container.document, "popup");
		var body = GetDocumentBody(document);

		container.style.pixelWidth = body.clientWidth * 50 / 100;
		container.style.pixelHeight = popupBody.scrollHeight;

		container.style.pixelLeft = (body.clientWidth - 2) < container.offsetWidth ? 0 : Math.min(container.style.pixelLeft, body.clientLeft + (body.offsetWidth - 2) - container.offsetWidth);
		container.style.pixelTop = (body.clientHeight - 2) < container.offsetHeight ? 0 : Math.min(container.style.pixelTop, body.clientTop + (body.clientHeight - 2) - container.offsetHeight);
*/
	}
}

/////////////////////////////////////////////////////////////////////////////

function GetElementById(doc, id)
{
	return typeof(doc.getElementById) != "undefined" ? doc.getElementById(id) : doc.all(id);
}

function GetDocumentBody(doc)
{
	return doc.compatMode == "CSS1Compat" ? doc.documentElement : doc.body;
}

function CancelEvent(evt)
{
	if (g_Browser.IsIE)
	{
		evt.returnValue = false;
	}
	else
	{
		evt.preventDefault();
	}
}
