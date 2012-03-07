#!/usr/bin/env python
# -*- coding: iso-8859-1 -*-
#****************************************************************************
#   mfGraph Library for manipulating and rendering DOT graphs               #
#   Copyright (c) 2005 Michael Foetsch                                      #
#                                                                           #
#  This library is free software; you can redistribute it and/or            #
#  modify it under the terms of the GNU Lesser General Public               #
#  License as published by the Free Software Foundation; either             #
#  version 2.1 of the License, or (at your option) any later version.       #
#                                                                           #
#  This library is distributed in the hope that it will be useful,          #
#  but WITHOUT ANY WARRANTY; without even the implied warranty of           #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU         #
#  Lesser General Public License for more details.                          #
#                                                                           #
#  You should have received a copy of the GNU Lesser General Public         #
#  License along with this library; if not, write to the Free Software      #
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  #
#                                                                           #
#  Contact:    foetsch@yahoo.com                                            #
#              http://mfgraph.sourceforge.net/                              #
#****************************************************************************

"""graph_drawer -

class GraphDrawer renders a pymfg.DrawGraph to a wx.DC
"""

from mfgraph import pymfg
import wx

# wxPython backward-compatibility

if hasattr(wx, "__version__") and wx.__version__ >= '2.6':
    WX_COMPATIBILITY_BASE_CLASS = object
else:
    # class _ReplacementObject is a workaround for a problem with wxPython 2.4:
    # GraphDrawer is a base class of GraphWindow, which is also derived from
    # wx.ScrolledWindow. Both GraphDrawer and GraphWindow use "property" and
    # must therefore be new-style classes (derived from "object"). However, deriving
    # from a wx class *and* being a new-style class does not work with
    # wxPython 2.4 (and earlier). _ReplacementObject emulates enough of
    # the property mechanism to work with GraphDrawer/GraphWindow without
    # being a new-style class.
    #
    class _ReplacementObject:
        def __setattr__(self, name, value):
            obj = _ReplacementObject.__GetClassAttribute(self.__class__, name)
            if hasattr(obj, "__set__"):
                return obj.__set__(self, value)
            else:
                self.__dict__[name] = value

        def __getattr__(self, name):
            obj = _ReplacementObject.__GetClassAttribute(self.__class__, name)
            if hasattr(obj, "__get__"):
                return obj.__get__(self)
            else:
                return self.__dict__[name]

        def __GetClassAttribute(cls, name):
            if hasattr(cls, name):
                return getattr(cls, name)
            else:
                for base in cls.__bases__:
                    attr = _ReplacementObject.__GetClassAttribute(base, name)
                    if attr is not None:
                        return attr
                else:
                    return None
        __GetClassAttribute = staticmethod(__GetClassAttribute)

    WX_COMPATIBILITY_BASE_CLASS = _ReplacementObject


class GraphDrawer(WX_COMPATIBILITY_BASE_CLASS):
    """@brief Render a pymfg.DrawGraph to a wx.DC

    Members:
    @li @c Zoom - zoom factor in percent of the original; default is 100
    @li @c CharacterEncoding - character encoding in which label texts
        should be interpreted; default is "ISO-8859-1"

    Usage Example:
    @code
    graph = pymfg.DrawGraph()
    graph.LoadFromXdot(xdot)
    drawer = GraphDrawer()
    drawer.SetDrawGraph(graph)
    drawer.Drawer(wx_dc, 0, 0)
    @endcode
    """


    # Static mappings that determine how to map pymfg drawing attributes
    # to wxPython.

    s_brush_styles = {
        pymfg.FILL_STYLE_SOLID: wx.SOLID,
        pymfg.FILL_STYLE_CLEAR: wx.TRANSPARENT,
        pymfg.FILL_STYLE_DIAG: wx.FDIAGONAL_HATCH,
        pymfg.FILL_STYLE_BACK_DIAG: wx.BDIAGONAL_HATCH,
        pymfg.FILL_STYLE_CROSS: wx.CROSS_HATCH,
        pymfg.FILL_STYLE_DIAG_CROSS: wx.CROSSDIAG_HATCH,
        pymfg.FILL_STYLE_HORZ: wx.HORIZONTAL_HATCH,
        pymfg.FILL_STYLE_VERT: wx.VERTICAL_HATCH
        }

    s_pen_styles = {
        pymfg.PEN_STYLE_SOLID: wx.SOLID,
        pymfg.PEN_STYLE_DASH: wx.LONG_DASH,
        pymfg.PEN_STYLE_DASH_DOT: wx.DOT_DASH,
        pymfg.PEN_STYLE_DASH_DOT_DOT: wx.SHORT_DASH,
        pymfg.PEN_STYLE_NULL: wx.TRANSPARENT
        }

    s_font_families = {
        pymfg.FONT_FAMILY_DECORATIVE: wx.DECORATIVE,
        pymfg.FONT_FAMILY_MODERN: wx.MODERN,
        pymfg.FONT_FAMILY_ROMAN: wx.ROMAN,
        pymfg.FONT_FAMILY_SCRIPT: wx.SCRIPT,
        pymfg.FONT_FAMILY_SWISS: wx.SWISS
        }

    s_font_styles = {
        pymfg.FONT_STYLE_REGULAR: (wx.NORMAL, wx.NORMAL),
        pymfg.FONT_STYLE_BOLD: (wx.NORMAL, wx.BOLD),
        pymfg.FONT_STYLE_ITALIC: (wx.ITALIC, wx.NORMAL),
        pymfg.FONT_STYLE_BOLD_ITALIC: (wx.ITALIC, wx.BOLD)
        }


    def __init__(self):
        """@brief Init."""
        self.__SetZoom(100)
        self.__SetCharacterEncoding("ISO-8859-1")
            # Don't use "self.Xy = xy" because this may lead to conflicts
            # with derived classes that redefine properties with the same name

        self.__m_draw_graph = None
        self.__m_width = None
        self.__m_height = None

    def SetDrawGraph(self, draw_graph):
        """@brief Set pymfg.DrawGraph that will be drawn.

        @param draw_graph - pymfg.DrawGraph object; may be None

        @exception KeyError - When the graph is empty (LoadFromXdot() hasn't
            been called), a KeyError is raised by GetGraphSizeUnzoomed(), because
            the "bb" attribute will not be found. An empty graph is not the
            only reason for a missing "bb" attribute, so this method lets
            the caller decide how to handle the exception. The caller should
            keep track of whether LoadFromXdot() has been called or not, and
            suppress the exception if it has.
        """
        self.__m_draw_graph = draw_graph
        if draw_graph is not None:
            self.__m_width, self.__m_height = self.GetGraphSizeUnzoomed()
        else:
            self.__m_width = self.__m_height = 0

    def Draw(self, dc, left, top):
        """@brief Render a pymfg.DrawGraph to a wx.DC at the specified position.

        @param dc - wx.DC; this method modifies DC attributes such as the
            current pen and the device origin
        @param left -
        @param top - @a left and @a top specify the upper-left corner of the
            rectangle to draw to
        """
        dc.SetUserScale(self.__m_zoom / 100.0, self.__m_zoom / 100.0)
        dc.SetAxisOrientation(True, True)
        width, height = self.GetGraphSizeZoomed()
        dc.SetDeviceOrigin(left, height + top)
        drawInfos = self.__m_draw_graph.GetDrawCmdList()
        for di in drawInfos:
            self.__DrawFromDrawInfo(dc, di)

    def LogicalPointToDevicePoint(self, pt):
        """@brief Apply the zoom factor (@c Zoom property) to a point
            and flip the y-coordinate

        Used to transform a point in graph units to device units.

        @param pt - pymfg.Point
        @return pymfg.Point
        """
        return pymfg.Point(int(pt.x * self.__m_zoom / 100.0),
                           int((self.__m_height - pt.y) * self.__m_zoom / 100.0))

    def DevicePointToLogicalPoint(self, pt):
        """@brief Reverse the effect of LogicalPointToDevicePoint().

        Used to transform a point in device units to graph units.

        @param pt - pymfg.Point
        @return pymfg.Point
        """
        return pymfg.Point(int(pt.x * 100.0 / self.__m_zoom),
                           int(self.__m_height - (pt.y * 100.0 / self.__m_zoom)))

    def LogicalRectToDeviceRect(self, rect):
        """@brief Apply the zoom factor (@c Zoom property) to a rectangle
            and the flip the y-coordinate

        Used to transform a rectangle in graph units to device units.

        @param pt - pymfg.Rect
        @return pymfg.Rect
        """
        upper_left = pymfg.Point(rect.left, rect.top);
        lower_right = pymfg.Point(rect.right, rect.bottom);
        zoomed_upper_left = self.LogicalPointToDevicePoint(upper_left);
        zoomed_lower_right = self.LogicalPointToDevicePoint(lower_right);
        zoomed_rect = pymfg.Rect(zoomed_upper_left.x,
                                 zoomed_upper_left.y,
                                 zoomed_lower_right.x,
                                 zoomed_lower_right.y);
        zoomed_rect.Normalize();
        return zoomed_rect;

    def GetGraphSizeZoomed(self):
        """@brief Return graph size with zoom factor applied.

        @return tuple of (width,height) with zoom factor (@c Zoom property) applied.
        """
        width, height = self.GetGraphSizeUnzoomed()
        zoomed_size = self.LogicalPointToDevicePoint(pymfg.Point(width, 0))
        return zoomed_size.x, zoomed_size.y

    def GetGraphSizeUnzoomed(self):
        """@brief Return graph size in graph units (i.e., at zoom factor 100).

        @return tuple of (width,height)
        """
        if self.__m_draw_graph is not None:
            bb = self.__m_draw_graph.GetTopLevel().Attribs()["bb"]
            x, y, width, height = [int(x) for x in bb.split(',')]
            return width, height
        else:
            return 0, 0

    def _ModifyDrawAttribs(self, di, brush, pen, font, font_color, transparent_brush):
        """@brief Modify drawing attributes immediately before rendering begins.

        Override in derived classes to apply special drawing attributes
        depending on the rendered object (e.g., to highlight "selected" objects).
        The default implementation returns the passed object unmodified.

        @param di - pymfg.DrawInfo object; use @c di.GetOwnerAsObject() to
            query information about the rendered object; see
            file "mfgraph-0.3/pymfg/mfg_draw_info.i" for available methods
        @param brush - wx.Brush
        @param pen - wx.Pen
        @param font - wx.Font
        @param font_color - wx.Colour for text
        @param transparent_brush - wx.Brush to use for unfilled shapes
            (by default, a brush with style wx.TRANSPARENT)
        @return tuple of modified or new @c (brush,pen,font,font_color,transparent_brush)
        """
        return brush, pen, font, font_color, transparent_brush

    def __DrawFromDrawInfo(self, dc, di):
        def SetTransparentBrush(transparent_brush):
            old_brush = dc.GetBrush()
            dc.SetBrush(transparent_brush)
            return old_brush

        num = di.GetNumCmds()
        if num > 0:
            transparent_brush = self.__ApplyDrawInfoAttribs(dc, di)
        for i in xrange(num):
            cmd_type = di.GetCmdType(i)
            cmd = di.GetCmd(i)
            if cmd_type == di.DRAW_CMD_ELLIPSE:
                cmd = cmd.AsEllipse()
                rect = cmd.GetCoords()
                if not cmd.IsFilled():
                    old_brush = SetTransparentBrush(transparent_brush)
                dc.DrawEllipse(rect.left, rect.top, rect.Width(), rect.Height())
                if not cmd.IsFilled():
                    dc.SetBrush(old_brush)
            elif cmd_type == di.DRAW_CMD_POLYGON:
                cmd = cmd.AsPolygon()
                pts = GraphDrawer.__GetPtList(cmd)
                if not cmd.IsFilled():
                    old_brush = SetTransparentBrush(transparent_brush)
                dc.DrawPolygon(pts)
                if not cmd.IsFilled():
                    dc.SetBrush(old_brush)
            elif cmd_type == di.DRAW_CMD_POLYLINE:
                cmd = cmd.AsPolyline()
                pts = GraphDrawer.__GetPtList(cmd)
                dc.DrawLines(pts)
            elif cmd_type == di.DRAW_CMD_BEZIER:
                cmd = cmd.AsBezier()
                pts = GraphDrawer.__GetPtList(cmd)
                dc.DrawSpline(pts)
            elif cmd_type == di.DRAW_CMD_TEXT:
                cmd = cmd.AsText()
                pos = cmd.GetPos()
                if hasattr(wx, "PlatformInfo") and "unicode" in wx.PlatformInfo:
                    text = cmd.GetText().decode(self.__m_character_encoding)
                else:
                    text = cmd.GetText()
                align = cmd.GetAlign()
                width, height, descent, external_leading = dc.GetFullTextExtent(text)
                if align == pymfg.ALIGN_CENTER:
                    x = pos.x - width / 2
                elif align == pymfg.ALIGN_RIGHT:
                    x = pos.x - width
                else:
                    x = pos.x
                y = pos.y + height - descent - external_leading
                dc.DrawText(text, x, y)
                if not cmd.BoundingRectHasBeenSet():
                    rect = pymfg.Rect(x, pos.y - descent,
                                      x + width, pos.y + height)
                    cmd.SetBoundingRect(rect)

    def __ApplyDrawInfoAttribs(self, dc, di):
        attribs = di.GetAttribs()
        brush_color = self.__MakeWxColor(attribs.mFillColor)
        brush_style = self.s_brush_styles.get(attribs.mFillStyle, wx.SOLID)
        brush = wx.Brush(brush_color, brush_style)

        pen_color = self.__MakeWxColor(attribs.mLineColor)
        pen_style = self.s_pen_styles.get(attribs.mPenStyle, wx.SOLID)
        pen = wx.Pen(pen_color, attribs.mPenWidth, pen_style)

        font_size = attribs.mFontSize
        try:
            font_size = font_size * 72 / dc.GetPPI()[1]
        except ZeroDivisionError:
            pass
        font_family = self.s_font_families.get(attribs.mFontFamily, wx.DEFAULT)
        font_style, font_weight = self.s_font_styles.get(attribs.mFontStyle,
                                                    (wx.NORMAL, wx.NORMAL))
        if attribs.mFontName:
            font_face_name = attribs.mFontName
        else:
            font_face_name = ""
        font_color = self.__MakeWxColor(attribs.mFontColor)
        font = wx.Font(font_size, font_family, font_style,
                       font_weight, faceName=font_face_name)

        transparent_brush = wx.Brush(wx.WHITE, wx.TRANSPARENT)

        brush, pen, font, font_color, transparent_brush \
               = self._ModifyDrawAttribs(di, brush, pen, font,
                                         font_color, transparent_brush)

        dc.SetBrush(brush)
        dc.SetPen(pen)
        dc.SetFont(font)
        dc.SetTextForeground(font_color)

        return transparent_brush

    def __MakeWxColor(self, pymfg_color):
        red = pymfg_color & 0xFF
        green = (pymfg_color & (0xFF << 8)) >> 8
        blue = (pymfg_color & (0xFF << 16)) >> 16
        return wx.Colour(red, green, blue)

    def __GetPtList(drawCmd):
        ret = []
        num = drawCmd.GetNumPts()
        for i in xrange(num):
            pt = drawCmd.GetPt(i)
            ret.append(wx.Point(pt.x, pt.y))
        return ret
    __GetPtList = staticmethod(__GetPtList)


    # Property accessors.

    def __GetZoom(self):
        return self.__m_zoom

    def __SetZoom(self, zoom):
        self.__m_zoom = max(1, zoom)

    def __GetCharacterEncoding(self):
        return self.__m_character_encoding

    def __SetCharacterEncoding(self, new):
        self.__m_character_encoding = new


    # Properties.

    Zoom = property(fget=__GetZoom, fset=__SetZoom)

    CharacterEncoding = property(fget=__GetCharacterEncoding,
                                 fset=__SetCharacterEncoding)

