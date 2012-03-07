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

from graph_drawer import GraphDrawer
from mfgraph import pymfg
from mfgraph.pymfg import deref
from object_selection import ObjectSelection
import wx
import sys
if sys.version_info < (2, 5):
    import sets
    set = sets.Set

class DotListingError(Exception):
    """@brief Base class for DOT-related exceptions."""
    pass

class InvalidDotListing(DotListingError):
    """@brief DOT listing could not be parsed.

    Possible cause: The file was some arbitrary text file and not in DOT format.
    """
    def __init__(self, dot_listing=""):
        if len(dot_listing) > 50:
            dot_listing = dot_listing[:50] + "<...>"
        Exception.__init__(self,
                           "Invalid DOT listing: %s\n  Not a DOT file?"
                               % repr(dot_listing))

class InvalidXdotListing(DotListingError):
    """@brief DOT listing does not contain XDOT attributes.

    Possible cause: The file *is* a DOT file but has not been layouted by
        DOT with the "-Txdot" argument.
    """
    def __init__(self, dot_listing=""):
        if len(dot_listing) > 50:
            dot_listing = dot_listing[:50] + "<...>"
        Exception.__init__(self,
                           "Not an XDOT listing: %s\n" \
                               "  Has DOT been called with '-Txdot'?"
                               % repr(dot_listing))

class GraphWindow(wx.ScrolledWindow,
                  GraphDrawer):
    """@brief wx.ScrolledWindow descendant that renders graphs

    Usage example:
    @code
    graph_window = MfgraphWindow(parent)
    graph_window.XdotListing = file("test.xdot").read()
    @endcode

    Properties:
    @li @c DrawGraph - (read-only) pymfg.DrawGraph object. You should *not*
        modify the graph object. If you do, you must use DrawGraph.PrintAsDot()
        to get a DOT listing that you can assign back to XdotListing (usually
        after converting to XDOT format by running the "dot" utility).
        May be None before XdotListing has been assigned.
    @li @c HighlightColor - RGB color triple (0 to 255) for highlighted
        graph objects (default = (0, 0, 255)). For finer control over how
        highlighted objects are rendered, you can overwrite _ModifyDrawAttribs().
    @li @c HighlightOpacity - how much of @c HighlightColor to add
        to the original color of the object; 0=min, 255=max; (default = 64)
    @li @c HighlightPenWidth - pen width for highlighted graph objects
    @li @c HitTestTolerance - hit-test tolerance in pixels (default = 3)
    @li @c Margin - margin to add around the graph, in pixels (default = 20)
    @li @c MultiSelect - bool that indicates whether multi-selection is
        allowed by Ctrl-clicking
    @li @c ScrollPageSize - pixels per scroll unit (default = 20)
    @li @c Selection - (read-only) ObjectSelection instance.
        The window is automatically refreshed when you change the selection
        through this object.
    @li @c XdotListing - XDOT file contents as string (default = None)
    @li @c Zoom - zoom factor in percent (default = 100)
    @li @c ZoomFactors - list of zoom factors from which to choose when mouse
        wheel is rolled or when numpad +/- are pressed. The @c Zoom property
        can be set independent of this list. When you set this property, the list
        is sorted and only entries > 0 are accepted
    @li @c ZoomFactorIndex - current index into @c ZoomFactors for
        zooming with mouse wheel or numpad +/-

    Derived properties:
    @li @c CharacterEncoding - set this to the character encoding of the XDOT
        file from which @c XdotListing was read
    """
    def __init__(self, parent, id=-1, pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.HSCROLL | wx.VSCROLL,
                 name="mfGraphWindow"):
        wx.ScrolledWindow.__init__(self, parent, id, pos, size, style, name)
        self.EnableScrolling(True, True)
        self.SetBackgroundColour(wx.WHITE)

        GraphDrawer.__init__(self)

        # events
        wx.EVT_PAINT(self, self.OnPaint)
        wx.EVT_KEY_DOWN(self, self.OnKeyDown)
        wx.EVT_LEFT_DOWN(self, self.OnLeftDown)
        wx.EVT_LEFT_DCLICK(self, self.OnLeftDoubleClick)
        wx.EVT_RIGHT_DOWN(self, self.OnRightDown)
        wx.EVT_MOUSEWHEEL(self, self.OnMouseWheel)

        # members
        self.__m_draw_graph = pymfg.DrawGraph()
        self.__m_selection \
            = ObjectSelection(selection_changed_callback=self.OnSelectionChanged)
        self.__m_accum_wheel_rotation = 0

        # properties
            # Don't use "self.PropertyName = xy" because this may lead to conflicts
            # with derived classes that redefine properties with the same name
        self.__SetHighlightOpacity(64)
        self.__SetHighlightColor((0, 0, 255))
        self.__SetHighlightPenWidth(2)
        self.__SetHitTestTolerance(3)
        self.__SetMargin(20)
        self.__SetMultiSelect(True)
        self.__SetScrollPageSize(20)
        self.__SetXdotListing(None)
        self.__SetZoom(100)
        self.__SetZoomFactors([10, 25, 50, 75, 100, 125, 150, 200, 300, 400, 500])
        self.__SetZoomFactorIndex(4)

    def OnPaint(self, event):
        dc = wx.PaintDC(self)
        self.PrepareDC(dc)
        dc.BeginDrawing()
        dc.SetBackground(wx.WHITE_BRUSH)
        dc.Clear()
        try:
            self.Draw(
                dc,
                -self.GetViewStart()[0] * self.__m_scroll_page_size + self.__m_margin,
                -self.GetViewStart()[1] * self.__m_scroll_page_size + self.__m_margin)
        except Exception, e:
            print "Exception in Paint:", e
        dc.EndDrawing()

    def OnKeyDown(self, event):
        if event.GetKeyCode() == wx.WXK_TAB:
            self.SelectNextObject(forward=not event.ShiftDown())
        elif event.GetKeyCode() == wx.WXK_NUMPAD_SUBTRACT:
            self.ZoomFactorIndex -= 1
        elif event.GetKeyCode() == wx.WXK_NUMPAD_ADD:
            self.ZoomFactorIndex += 1
        else:
            event.Skip()

    def OnLeftDown(self, event):
        if self.__m_draw_graph is not None:
            obj = self.GetObjectAtPoint(event.GetX(), event.GetY())
            self.OnObjectClicked(obj, event)
        event.Skip()

    def OnLeftDoubleClick(self, event):
        if self.__m_draw_graph is not None:
            obj = self.GetObjectAtPoint(event.GetX(), event.GetY())
            self.OnObjectDoubleClicked(obj, event)
        event.Skip()

    def OnRightDown(self, event):
        if self.__m_draw_graph is not None:
            obj = self.GetObjectAtPoint(event.GetX(), event.GetY())
            self.OnObjectRightClicked(obj, event)
        event.Skip()

    def OnMouseWheel(self, event):
        """@brief Event handler for EVT_MOUSEWHEEL, used for zooming."""
        rotation = event.GetWheelRotation()
        if (rotation > 0 and self.__m_accum_wheel_rotation < 0) \
           or (rotation < 0 and self.__m_accum_wheel_rotation > 0):
            # when user starts to roll into a new direction,
            # discard what we have accumulated in the other direction
            self.__m_accum_wheel_rotation = 0
        self.__m_accum_wheel_rotation += rotation
        rotation_units = self.__m_accum_wheel_rotation / event.GetWheelDelta()
        if abs(rotation_units) >= 1:
            self.ZoomFactorIndex -= rotation_units
            self.__m_accum_wheel_rotation = 0

    def OnObjectClicked(self, obj, event):
        """@brief Invoked if a graph object has been clicked with the
            left mouse button.

        Default behavior is to set the clicked object as selected.
        If the background has been clicked or if the clicked object's
        type is not in Selection.SelectableObjectTypes, all objects
        are unselected. If Ctrl is pressed, the selection state of the
        clicked object is toggled.

        @param obj - pymfg.Object or None if background has been clicked
        @param event - wx.MouseEvent
        """
        if event.ControlDown() and self.__m_multi_select:
            if obj is not None:
                self.__m_selection.SetObjectSelected(
                    obj,
                    not self.__m_selection.IsObjectSelected(obj))
        else:
            self.__m_selection.SelectSingleObject(obj)

    def OnObjectDoubleClicked(self, obj, event):
        """@brief Invoked if a graph object has been double-clicked with the
            left mouse button.

        Can be overwritten in derived classes. The default implementation
        does nothing.

        @param obj - pymfg.Object or None if background has been clicked
        @param event - wx.MouseEvent
        """
        pass

    def OnObjectRightClicked(self, obj, event):
        """@brief Invoked if a graph object has been clicked with the
            right mouse button.

        Default behavior is to set the clicked object as the only
        selected object if it is not already part of the selection.
        If the background has been clicked or if the clicked object's
        type is not in Selection.SelectableObjectTypes, nothing happens.

        @param obj - pymfg.Object or None if background has been clicked
        @param event - wx.MouseEvent
        """
        if obj is None or not self.__m_selection.IsObjectSelectable(obj):
            return
        if not self.__m_selection.IsObjectSelected(obj):
            self.__m_selection.SelectSingleObject(obj)

    def OnSelectionChanged(self):
        self.Refresh(eraseBackground=True)

    def GetObjectAtPoint(self, x, y):
        """@brief Return pymfg.Object at specified device coordinates.

        @param x, y - Point in device coordinates (e.g., as returned by
            mouse events)
        @return pymfg.Object or None if no object exists at the specified point
        """
        x, y = self.CalcUnscrolledPosition(x - self.__m_margin,
                                           y - self.__m_margin)
        logical_point = self.DevicePointToLogicalPoint(pymfg.Point(x, y))
        draw_info = self.__m_draw_graph.HitTest(logical_point,
                                                None,
                                                self.__m_hit_test_tolerance)
        if draw_info is not None:
            return draw_info.GetOwnerAsObject()
        else:
            return None

    def GetViewRect(self):
        """@brief Return the rectangle in zoomed graph coordinates
            that is displayed in the window.

        @return pymfg.Rect
        """
        view_left = self.GetViewStart()[0] * self.__m_scroll_page_size
        view_top = self.GetViewStart()[1] * self.__m_scroll_page_size
        view_right = view_left + self.GetClientSizeTuple()[0]
        view_bottom = view_top + self.GetClientSizeTuple()[1]
        view_rect = pymfg.Rect(view_left, view_top, view_right, view_bottom)
        view_rect.Offset(-self.__m_margin, -self.__m_margin)
        return view_rect

    def SetViewRect(self, rect):
        """@brief Scroll the window so that the specified rectangle, given
            in zoomed graph coordinates, is displayed.

        @param rect - pymfg.Rect

        @remarks
        The window's width and height are not modified. If @a rect is larger
        than the window size, the window is scrolled to the upper left corner
        of @a rect.
        """
        view_rect = self.GetViewRect()
        if rect.right > view_rect.right:
            view_rect.Offset(rect.right - view_rect.right, 0)
        if rect.left < view_rect.left:
            view_rect.Offset(rect.left - view_rect.left, 0)
        if rect.bottom > view_rect.bottom:
            view_rect.Offset(0, rect.bottom - view_rect.bottom)
        if rect.top < view_rect.top:
            view_rect.Offset(0, rect.top - view_rect.top)
        self.Scroll((view_rect.left + self.__m_margin) / self.__m_scroll_page_size,
                    (view_rect.top + self.__m_margin) / self.__m_scroll_page_size)

    def ScrollToObject(self, obj):
        """@brief Scroll the window so that the specified graph object is visible.

        @param obj - pymfg.Object

        @remarks
        If the dimensions of @a obj are larger than the window size, the window is
        scrolled to the upper left corner of @a obj.
        """
        obj_rect = self.LogicalRectToDeviceRect(obj.AsDrawObject().GetBoundingRect())
        obj_rect.Inflate(self.__m_margin, self.__m_margin)
        self.SetViewRect(obj_rect)

    def SelectNextObject(self, forward=True):
        """@brief When TAB is pressed, move selection to next graph object.

        @param forward - True for TAB, False for Shift+TAB
        """
        first_obj = self.__m_selection.GetFirstSelectedObject()
        if forward:
            next_obj = self.__m_selection.GetFollowingObject(first_obj)
        else:
            next_obj = self.__m_selection.GetPreviousObject(first_obj)
        self.__m_selection.SelectSingleObject(next_obj)
        self.ScrollToObject(next_obj)

    def _ModifyDrawAttribs(self, di, brush, pen, font, font_color, transparent_brush):
        """@brief Set drawing attributes for "selected" objects.

        @sa
            GraphDrawer
        """
        if di.GetOwnerAsObject().this in map(lambda o: o.this,
                                             self.__m_selection.SelectedObjects):
            brush.SetColour(AddColorTint(brush.GetColour(),
                                         self.__m_highlight_opacity,
                                         self.__m_highlight_color))
            pen.SetColour(AddColorTint(pen.GetColour(),
                                       self.__m_highlight_opacity,
                                       self.__m_highlight_color))
            pen.SetWidth(self.HighlightPenWidth)
            font.SetUnderlined(True)
            font_color = AddColorTint(font_color,
                                      self.__m_highlight_opacity,
                                      self.__m_highlight_color)
            transparent_brush.SetColour(AddColorTint(self.GetBackgroundColour(),
                                                     self.__m_highlight_opacity,
                                                     self.__m_highlight_color))
            transparent_brush.SetStyle(wx.SOLID)
        return brush, pen, font, font_color, transparent_brush

    def __AdjustScrollbarsToGraphSize(self):
        width, height = self.__GetGraphSize()
        self.SetScrollbars(self.__m_scroll_page_size,
                           self.__m_scroll_page_size,
                           (width + 2 * self.__m_margin) / self.__m_scroll_page_size,
                           (height + 2 * self.__m_margin) / self.__m_scroll_page_size)

    def __GetGraphSize(self):
        try:
            return self.GetGraphSizeZoomed()
        except KeyError:
            if self.__m_xdot_listing is None:
                return 0, 0
            else:
                raise


    # Property Accessors.

    def __GetDrawGraph(self):
        return self.__m_draw_graph

    def __GetHighlightOpacity(self):
        return self.__m_highlight_opacity

    def __SetHighlightOpacity(self, new):
        self.__m_highlight_opacity = new

    def __GetHighlightColor(self):
        return self.__m_highlight_color

    def __SetHighlightColor(self, (red, green, blue)):
        self.__m_highlight_color = (red, green, blue)

    def __GetHighlightPenWidth(self):
        return self.__m_highlight_pen_width

    def __SetHighlightPenWidth(self, new):
        self.__m_highlight_pen_width = new

    def __GetMargin(self):
        return self.__m_margin

    def __SetMargin(self, new):
        self.__m_margin = new

    def __GetMultiSelect(self):
        return self.__m_multi_select

    def __SetMultiSelect(self, new):
        self.__m_multi_select = new

    def __GetHitTestTolerance(self):
        return self.__m_hit_test_tolerance

    def __SetHitTestTolerance(self, new):
        self.__m_hit_test_tolerance = new

    def __GetScrollPageSize(self):
        return self.__m_scroll_page_size

    def __SetScrollPageSize(self, new):
        self.__m_scroll_page_size = new
        self.SetScrollRate(new, new)

    def __GetSelection(self):
        return self.__m_selection

    def __GetXdotListing(self):
        return self.__m_xdot_listing

    def __SetXdotListing(self, new):
        self.__m_xdot_listing = new
        if self.__m_xdot_listing is not None:
            self.__m_draw_graph = pymfg.DrawGraph()
            if not self.__m_draw_graph.LoadFromXdot(self.__m_xdot_listing):
                raise InvalidDotListing(new)
        else:
            self.__m_draw_graph = None
        try:
            self.SetDrawGraph(self.__m_draw_graph)
        except KeyError:
            raise InvalidXdotListing(new)
        self.__AdjustScrollbarsToGraphSize()
        self.__m_selection \
            = ObjectSelection(selection_changed_callback=self.OnSelectionChanged,
                              draw_graph=self.__m_draw_graph)
        self.Refresh(eraseBackground=True)

    def __GetZoom(self):
        return GraphDrawer.Zoom.__get__(self)

    def __SetZoom(self, new):
        if new == self.Zoom:
            return
        old_pt = self.DevicePointToLogicalPoint(pymfg.Point(
            self.GetViewStart()[0] * self.__m_scroll_page_size,
            self.GetViewStart()[1] * self.__m_scroll_page_size))
        GraphDrawer.Zoom.__set__(self, new)
        self.__AdjustScrollbarsToGraphSize()
        new_pt = self.LogicalPointToDevicePoint(old_pt)
        self.Scroll(new_pt.x / self.__m_scroll_page_size,
                    new_pt.y / self.__m_scroll_page_size)
        self.Refresh(eraseBackground=True)

    def __GetZoomFactors(self):
        return self.__m_zoom_factors[:]

    def __SetZoomFactors(self, new):
        new = list(set(filter(lambda x: x > 0, new)))
        new.sort()
        self.__m_zoom_factors = new

    def __GetZoomFactorIndex(self):
        return self.__m_zoom_factor_index

    def __SetZoomFactorIndex(self, new):
        self.__m_zoom_factor_index = max(0, min(len(self.__m_zoom_factors) - 1, new))
        self.Zoom = self.__m_zoom_factors[self.__m_zoom_factor_index]


    # Properties.

    DrawGraph = property(fget=__GetDrawGraph)

    HitTestTolerance = property(fget=__GetHitTestTolerance,
                                fset=__SetHitTestTolerance)

    HighlightColor = property(fget=__GetHighlightColor,
                              fset=__SetHighlightColor)

    HighlightOpacity = property(fget=__GetHighlightOpacity,
                                fset=__SetHighlightOpacity)

    HighlightPenWidth = property(fget=__GetHighlightPenWidth,
                                 fset=__SetHighlightPenWidth)

    Margin = property(fget=__GetMargin, fset=__SetMargin)

    ScrollPageSize = property(fget=__GetScrollPageSize,
                              fset=__SetScrollPageSize)

    Selection = property(fget=__GetSelection)

    XdotListing = property(fget=__GetXdotListing,
                           fset=__SetXdotListing)

    Zoom = property(fget=__GetZoom,
                    fset=__SetZoom)

    ZoomFactors = property(fget=__GetZoomFactors,
                           fset=__SetZoomFactors)

    ZoomFactorIndex = property(fget=__GetZoomFactorIndex,
                               fset=__SetZoomFactorIndex)



def AddColorTint(wx_color, tint_opacity=0, tint_color=(0, 0, 0)):
    """@brief Add a color tint to a wx.Colour object.

    @param wx_color - wx.Colour object
    @param tint_opacity - how much of the tint color to add; 0=min, 255=max
    @param tint_color - tint color as RGB triple (0 to 255)
    @return New tinted wx.Colour object
    """
    red = wx_color.Red() + (tint_opacity
                          * (tint_color[0] - wx_color.Red()) + 128) / 255
    green = wx_color.Green() + (tint_opacity
                              * (tint_color[1] - wx_color.Green()) + 128) / 255
    blue = wx_color.Blue() + (tint_opacity
                              * (tint_color[2] - wx_color.Blue()) + 128) / 255
    return wx.Colour(red, green, blue)



if __name__ == "__main__":
    import os

    class MyFrame(wx.Frame):
        def __init__(self):
            wx.Frame.__init__(self, None, -1, "pymfg Example")

            self.m_graph_window = GraphWindow(self,
                                              -1,
                                              style=wx.SUNKEN_BORDER)
            self.m_graph_window.XdotListing = file(os.path.join('..',
                                                                'contrib',
                                                                'test.xdot')).read()
            self.m_graph_window.SetFocus()

    class MyApp(wx.PySimpleApp):
        def OnInit(self):
            self.frame = MyFrame()
            self.frame.Show()
            self.SetTopWindow(self.frame)
            return True

    app = MyApp()
    app.MainLoop()
