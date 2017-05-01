import React, { Component } from 'react'
import {
    View,
    Text,
    TouchableOpacity,
    Image,
    Button,
    PixelRatio,
    StyleSheet,
    NativeModules,
} from 'react-native'
import ImagePicker from 'react-native-image-picker'

class App extends Component {
    constructor(props) {
        super(props)
        this.state = { source: null, imageData: null }
        this.selectPhoto = this.selectPhoto.bind(this)
        this.dehazePhoto = this.dehazePhoto.bind(this)
    }

    selectPhoto() {
        const options = {
            quality: 1.0,
            maxWidth: 500,
            maxHeight: 500,
            storageOptions: {
                skipBackup: true,
                path: 'images',
            }
        }
        ImagePicker.showImagePicker(options, (response) => {
            console.log('Response = ', response)
            if (response.didCancel) {
                console.log('User cancelled photo picker')
            }
            else if (response.error) {
                console.log('ImagePicker Error: ', response.error)
            }
            else if (response.customButton) {
                console.log('User tapped custom button: ', response.customButton)
            }
            else {
                const { uri } = response
                this.setState({ source: { uri } })
            }
        })
    }

    async dehazePhoto() {
        const { source: { uri } } = this.state
        console.log(uri)
        const dehazedUri = uri && await NativeModules.Dehaze.init(uri)
        console.log(dehazedUri)
        this.setState({ source: { uri: dehazedUri } })
    }

    render() {
        const { source, imageData } = this.state
        return (
            <View style={styles.container}>
                <TouchableOpacity onPress={this.selectPhoto}>
                    <View style={[styles.avatar, styles.avatarContainer]}>
                        {source ? <Image source={source} /> : <Text>Select a Photo</Text>}
                    </View>
                </TouchableOpacity>
                <Button
                    title="Dehaze"
                    onPress={this.dehazePhoto}
                />
            </View>
        )
    }
}

const styles = StyleSheet.create({
    container: {
        flex: 1,
        justifyContent: 'center',
        alignItems: 'center',
        backgroundColor: '#F5FCFF',
    },
    avatarContainer: {
        borderColor: '#9B9B9B',
        borderWidth: 1 / PixelRatio.get(),
        justifyContent: 'center',
        alignItems: 'center'
    },
    avatar: {
        borderRadius: 75,
        width: 150,
        height: 150
    }
});

export default App
