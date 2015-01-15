Angular - Drag and Drop
=================

Lightweight drag and drop with angular using directives and HTML5

2.6kb minified, 0,54kb gzipped

See <a href="https://github.com/fisshy/angular-drag-drop/blob/master/example/index.html" alt="example">example</a> page for setup

Install
-------
With bower:

    $ bower install angular-dragndrop

</ul>

### Controller
```js
angular.module('myApp', ['dragAndDrop'])
.controller('MyCntrl', function($scope){

	$scope.cars = [ 
		{ name : 'Volvo' 	}, 
		{ name : 'Audi' 	}, 
		{ name : 'BMW' 		}, 
		{ name : 'Mercedes'	} 
	];

	$scope.sold = [ { name : 'Volvo' } ]; 

	$scope.moveToSold = function(car, element) {
		var index = $scope.cars.indexOf(car);
		$scope.cars.splice(index, 1);
		$scope.sold.push(car);
	};

});
```

### Drop
```html
<div drop="moveToSold">
	<div ng-repeat="car in sold">{{ car.name }}</div>
<div>
```

<b>Options</b><br>
drop  		- Takes a function thats called when drag is dropped<br/>
enter 		- Takes a function thats called when drag enters drop-area<br/>
leave 		- Takes a function thats called when drag leaves drop-area<br/>

    
### Drag
```html
<div ng-repeat="car in cars" drag ng-model="car">{{ car.name }}<div>
```

<b>Options</b><br>
ng-model  * - Context of the current drag item.<br/>
start 		- Takes a function to be called when drag starts<br />
end   		- Takes a function to be called when drag ends<br/>

<b>Exampel of usage</b>
<a target='_blank' href='http://imageshack.us/photo/my-images/268/angulardnd.png/'><img src='http://img268.imageshack.us/img268/4500/angulardnd.png' border='0'/></a><br></a>

### CSS
```css
Drag element
.drag

When drag element hovers another drag element
.drag.hover

When .drag is being dragged
.drag.dragging

Drop area
.drop

When .drag is being dragged appended to .drop
.drop.dropable

When drag element hovers a drop element
.drop.dropable.hover
```


Building
-------
	$ grunt
	
