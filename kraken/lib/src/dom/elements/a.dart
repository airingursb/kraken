/*
 * Copyright (C) 2019-present Alibaba Inc. All rights reserved.
 * Author: Kraken Team.
 */

import 'dart:ffi';
import 'package:kraken/bridge.dart';
import 'package:kraken/dom.dart';
import 'package:kraken/kraken.dart';
import 'package:kraken/module.dart';
import 'package:flutter/rendering.dart';

const String ANCHOR = 'A';

class AnchorElement extends Element {
  String? _href;
  String? _target;

  final Pointer<NativeAnchorElement> nativeAnchorElement;

  AnchorElement(int targetId, this.nativeAnchorElement, ElementManager elementManager)
      : super(targetId, nativeAnchorElement.ref.nativeElement, elementManager, tagName: ANCHOR) {
    addEvent(EVENT_CLICK);
  }

  String get pathname {
    if (_href != null) {
      return Uri.parse(_href!).path;
    } else {
      return '';
    }
  }

  @override
  void handleMouseEvent(String eventType, { PointerDownEvent? down, PointerUpEvent? up }) {
    super.handleMouseEvent(eventType, down: down, up: up);

    String? href = _href;
    if (href != null) {
      Uri sourceUri = Uri.parse(elementManager.controller.href);
      Uri resolvedUri = elementManager.controller.uriParser!.resolve(sourceUri, Uri.parse(href));
      elementManager.controller.view.handleNavigationAction(
          sourceUri.toString(), resolvedUri.toString(), _getNavigationType(resolvedUri.scheme));
    }
  }

  KrakenNavigationType _getNavigationType(String scheme) {
    switch (scheme.toLowerCase()) {
      case 'http':
      case 'https':
      case 'file':
        if (_target == null || _target == '_self') {
          return KrakenNavigationType.reload;
        }
    }

    return KrakenNavigationType.navigate;
  }

  @override
  getProperty(String key) {
    switch (key) {
      case 'pathname':
        return pathname;
    }
    return super.getProperty(key);
  }

  @override
  void setProperty(String key, dynamic value) {
    super.setProperty(key, value);
    switch (key) {
      case 'href':
        _href = value;
        break;
      case 'target':
        _target = value;
        break;
      default:
    }
  }

  @override
  void removeProperty(String key) {
    super.removeProperty(key);

    switch (key) {
      case 'href':
        _href = null;
        break;
      case 'target':
        _target = null;
        break;
      default:
    }
  }
}
