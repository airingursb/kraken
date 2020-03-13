it('background', () => {
  return new Promise((resolve) => {
    function setStyle(dom, object) {
      for (let key in object) {
        if (object.hasOwnProperty(key)) {
          dom.style[key] = object[key];
        }
      }
    }
    const div = document.createElement('div');
    setStyle(div, {
      width: '200px',
      height: '200px',
      backgroundColor: 'red'
    });


    // repeat
    const repeat = document.createElement('div');
    setStyle(repeat, {
      width: '100vw',
      height: '200px',
      marginTop: '10px',
      display: 'flex',
      flexDirection: 'row'
    });

    const div1 = document.createElement('div');
    setStyle(div1, {
      width: '100px',
      height: '200px',
      backgroundImage: 'url(https://img.alicdn.com/tfs/TB1H2Kcb1H2gK0jSZFEXXcqMpXa-70-72.png)',
      backgroundRepeat: 'no-repeat'
    });
    repeat.appendChild(div1);

    const div2 = document.createElement('div');
    setStyle(div2, {
      width: '100px',
      height: '200px',
      backgroundImage: 'url(https://img.alicdn.com/tfs/TB1H2Kcb1H2gK0jSZFEXXcqMpXa-70-72.png)',
      backgroundRepeat: 'repeat-x'
    });
    repeat.appendChild(div2);

    const div3 = document.createElement('div');
    setStyle(div3, {
      width: '100px',
      height: '200px',
      backgroundImage: 'url(https://img.alicdn.com/tfs/TB1H2Kcb1H2gK0jSZFEXXcqMpXa-70-72.png)',
      backgroundRepeat: 'repeat-y'
    });
    repeat.appendChild(div3);
    
    const div4 = document.createElement('div');
    setStyle(div4, {
      width: '100px',
      height: '200px',
      backgroundImage: 'url(https://img.alicdn.com/tfs/TB1H2Kcb1H2gK0jSZFEXXcqMpXa-70-72.png)',
      backgroundRepeat: 'repeat'
    });
    repeat.appendChild(div4);

    // position
    const position = document.createElement('div');
    setStyle(position, {
      width: '100vw',
      height: '200px',
      marginTop: '10px',
      display: 'flex',
      flexDirection: 'row'
    });

    const position1 = document.createElement('div');
    setStyle(position1, {
      width: '100px',
      height: '200px',
      backgroundImage: 'url(https://img.alicdn.com/tfs/TB1H2Kcb1H2gK0jSZFEXXcqMpXa-70-72.png)',
      backgroundPosition: 'center'
    });
    position.appendChild(position1);

    const position2 = document.createElement('div');
    setStyle(position2, {
      width: '100px',
      height: '200px',
      backgroundImage: 'url(https://img.alicdn.com/tfs/TB1H2Kcb1H2gK0jSZFEXXcqMpXa-70-72.png)',
      backgroundPosition: 'left'
    });
    position.appendChild(position2);

    const position3 = document.createElement('div');
    setStyle(position3, {
      width: '100px',
      height: '200px',
      backgroundImage: 'url(https://img.alicdn.com/tfs/TB1H2Kcb1H2gK0jSZFEXXcqMpXa-70-72.png)',
      backgroundPosition: 'top'
    });
    position.appendChild(position3);

    const position4 = document.createElement('div');
    setStyle(position4, {
      width: '100px',
      height: '200px',
      backgroundImage: 'url(https://img.alicdn.com/tfs/TB1H2Kcb1H2gK0jSZFEXXcqMpXa-70-72.png)',
      backgroundPosition: 'right'
    });
    position.appendChild(position4);

    const position5 = document.createElement('div');
    setStyle(position5, {
      width: '100px',
      height: '200px',
      backgroundImage: 'url(https://img.alicdn.com/tfs/TB1H2Kcb1H2gK0jSZFEXXcqMpXa-70-72.png)',
      backgroundPosition: 'bottom'
    });
    position.appendChild(position5);

    document.body.appendChild(div);
    document.body.appendChild(repeat);
    document.body.appendChild(position);

    requestAnimationFrame(() => {
      setTimeout(() => {
        resolve();
      }, 1100);
    });
  });
});